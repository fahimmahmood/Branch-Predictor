#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;


class BimodalPredictorForHybrid {
private:
    vector<int> table;
    int prediction_threshold;   // if counter >= this value => predict taken
    int counter_max;           // saturates at this
    int table_bits;            // M
    int table_mask;            // (1 << M) - 1

public:
BimodalPredictorForHybrid(int m)
        : prediction_threshold(4),
          counter_max(7),
          table_bits(m),
          table_mask((1 << m) - 1)
    {
        // Initialize all counters to 4 (weakly taken)
        table.resize(1 << table_bits, 4);
    }

    // Compute index from PC address
    int getIndex(const string& pc_address) const {
        unsigned long pc_val = stoul(pc_address, nullptr, 16);
        // Discard lowest 2 bits => pc_val >> 2, then mask out M bits
        return (pc_val >> 2) & table_mask;
    }

    // Return "t" or "n" depending on the table’s counter
    string getPrediction(const string& pc_address) {
        int idx = getIndex(pc_address);
        return (table[idx] >= prediction_threshold) ? "t" : "n";
    }

    // Update the saturating counter
    void update(const string& pc_address, const string& actual) {
        int idx = getIndex(pc_address);
        if (actual == "t") {
            table[idx] = min(counter_max, table[idx] + 1);
        } else {
            table[idx] = max(0, table[idx] - 1);
        }
    }

    // Print final table contents
    void printFinalContents() const {
        cout << "FINAL BIMODAL CONTENTS\n";
        for (int i = 0; i < (int)table.size(); i++) {
            cout << i << "\t" << table[i] << "\n";
        }
    }
};


class GSharePredictorForHybrid {
private:
    vector<int> table;
    int prediction_threshold;
    int counter_max;

    int global_history;            // The GHR (n bits, but store in an int)
    int history_bits;              // N
    int pc_bits;                   // M
    int pc_mask;                   // (1 << M) - 1

public:
GSharePredictorForHybrid(int m, int n)
        : prediction_threshold(4),
          counter_max(7),
          global_history(0),
          history_bits(n),
          pc_bits(m),
          pc_mask((1 << m) - 1)
    {
        // Initialize all counters to 4 (weakly taken)
        table.resize(1 << pc_bits, 4);
    }

    // Compute the index from the PC plus XOR with GHR
    int getIndex(const string& pc_address) const {
        unsigned long pc_val = stoul(pc_address, nullptr, 16);
        int pc_index = (pc_val >> 2) & pc_mask;
        if (history_bits == 0) {
            // If n = 0, it behaves like plain Bimodal
            return pc_index;
        }
        int maskGHR = (1 << history_bits) - 1;  // lower n bits
        int ghr_val = global_history & maskGHR;
        return pc_index ^ ghr_val;
    }

    // Return "t" or "n" depending on the table’s counter
    string getPrediction(const string& pc_address) {
        int idx = getIndex(pc_address);
        return (table[idx] >= prediction_threshold) ? "t" : "n";
    }

    // Update the saturating counter
    void update(const string& pc_address, const string& actual) {
        int idx = getIndex(pc_address);
        if (actual == "t") {
            table[idx] = min(counter_max, table[idx] + 1);
        } else {
            table[idx] = max(0, table[idx] - 1);
        }
    }

    // Update global history: shift right and put new outcome in MSB
    void updateHistory(const string& actual) {
        if (history_bits == 0) return; // no history to maintain
        // shift right
        global_history >>= 1;
        // if taken, set the top bit
        if (actual == "t") {
            global_history |= (1 << (history_bits - 1));
        }
    }

    // Print final table contents
    void printFinalContents() const {
        cout << "FINAL GSHARE CONTENTS\n";
        for (int i = 0; i < (int)table.size(); i++) {
            cout << i << "\t" << table[i] << "\n";
        }
    }
};

// --------------------------------------------------------------
// HybridPredictor
//
//  • We store a chooser table of size 2^K, each a 2-bit counter (0..3).
//  • Initialize all chooser counters to 1 (weakly prefer Bimodal).
//  • If chooser >= 2 => pick GShare's prediction, else pick Bimodal's.
//  • We update whichever predictor was chosen, *always* update GShare’s
//    global history, then update the chooser based on who was correct.
//
// States for chooser_table[i]:
//  0,1 => prefer Bimodal
//  2,3 => prefer GShare
// --------------------------------------------------------------
class HybridPredictor {
private:
    GSharePredictorForHybrid gshare;   // uses M1, N
    BimodalPredictorForHybrid bimodal; // uses M2

    vector<int> chooser;      // 2-bit saturating counters: 0..3
    int chooser_bits;         // K
    int chooser_mask;         // (1<<K) - 1

    int mispredictions;
    int num_predictions;
    string final_pred;

public:
    HybridPredictor(int K, int M1, int N, int M2)
        : gshare(M1, N),
          bimodal(M2),
          chooser_bits(K),
          chooser_mask((1 << K) - 1),
          mispredictions(0),
          num_predictions(0)
    {
        chooser.resize(1 << chooser_bits, 1); // init to 1 (weakly prefer Bimodal)
    }

    void predict(const string& pc_address, const string& actual) {
        // Step (1): get separate predictions from GShare, Bimodal
        string gshare_pred  = gshare.getPrediction(pc_address);
        string bimodal_pred = bimodal.getPrediction(pc_address);

        // Step (2): index into chooser table
        unsigned long pc_val = stoul(pc_address, nullptr, 16);
        // discard lowest 2 bits, then mask out K bits
        int chooser_index = (pc_val >> 2) & chooser_mask;

        // Step (3): final prediction based on chooser
        int cval = chooser[chooser_index];
        final_pred = (cval >= 2) ? gshare_pred : bimodal_pred;

        // Count misprediction
        num_predictions++;
        if (final_pred != actual) {
            mispredictions++;
        }

        // Step (4): update the selected predictor’s table
        if (cval >= 2) {
            // chose GShare
            gshare.update(pc_address, actual);
        } else {
            // chose Bimodal
            bimodal.update(pc_address, actual);
        }

        // Step (5): always update GShare’s global history
        gshare.updateHistory(actual);

        // Step (6): update the chooser if GShare and Bimodal differ in correctness
        bool g_correct  = (gshare_pred == actual);
        bool b_correct  = (bimodal_pred == actual);

        // If both correct or both incorrect => no change
        // If GShare correct, Bimodal incorrect => increment
        // If Bimodal correct, GShare incorrect => decrement
        if (g_correct && !b_correct) {
            chooser[chooser_index] = min(3, chooser[chooser_index] + 1);
        } else if (!g_correct && b_correct) {
            chooser[chooser_index] = max(0, chooser[chooser_index] - 1);
        }
        // otherwise no change
    }

    void generate_val_traces(const string& cmd) {
        cout << "COMMAND\n";
        cout << cmd << "\n";
        cout << "OUTPUT\n";
        cout << "number of Predictions: " << num_predictions << "\n";
        cout << "number of Mispredictions: " << mispredictions << "\n";

        double mr = 0.0;
        if (num_predictions > 0) {
            mr = (100.0 * (double)mispredictions / (double)num_predictions);
        }
        cout << fixed << setprecision(2);
        cout << "misprediction Rate: " << mr << "%\n";

        // Print final chooser contents
        cout << "FINAL CHOOSER CONTENTS\n";
        for (int i = 0; i < (int)chooser.size(); i++) {
            cout << i << "\t" << chooser[i] << "\n";
        }

        // Then GShare, then Bimodal
        gshare.printFinalContents();
        bimodal.printFinalContents();
    }
    string getLastPrediction() const {
        return final_pred;
    }
};

// --------------------------------------------------------------
// main() for the hybrid simulator
// Usage: sim hybrid <K> <M1> <N> <M2> <tracefile>
// --------------------------------------------------------------
// int main(int argc, char* argv[]) {
//     if (argc < 6) {
//         cout << "Usage: sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
//         return 1;
//     }

//     // parse command-line arguments
//     string branch_pred_type = argv[1];  // should be "hybrid"
//     int K  = stoi(argv[2]);
//     int M1 = stoi(argv[3]);
//     int N  = stoi(argv[4]);
//     int M2 = stoi(argv[5]);
//     string filename = argv[6];

//     // build command string for reporting
//     string cmd = "sim " + branch_pred_type + " " +
//                  to_string(K) + " " +
//                  to_string(M1) + " " +
//                  to_string(N) + " " +
//                  to_string(M2) + " " +
//                  filename;

//     // open trace file
//     ifstream traceFile;
//     traceFile.open("traces/" + filename);

//     // Create HybridPredictor
//     HybridPredictor predictor(K, M1, N, M2);

//     if (traceFile.is_open()) {
//         string line;
//         while (getline(traceFile, line)) {
//             stringstream ss(line);
//             vector<string> address_and_label;
//             string token;
//             while (ss >> token) {
//                 address_and_label.push_back(token);
//             }
//             if (address_and_label.size() == 2) {
//                 // address_and_label[0] = PC (hex), [1] = "t" or "n"
//                 predictor.predict(address_and_label[0], address_and_label[1]);
//             }
//         }
//         traceFile.close();

//         // Print results
//         predictor.generate_val_traces(cmd);
//     }
//     else {
//         cout << "Unable to open trace file\n";
//     }

//     return 0;
// }