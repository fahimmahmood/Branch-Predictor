#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include "bimodal.cpp"  // or place the bimodal class definition here
#include "gshare.cpp"   // or place the gshare class definition here
#include "hybrid.cpp"   // or place the hybrid class definition here

using namespace std;

int main(int argc, char* argv[])
{
    // Check if at least the predictor type is given
    if (argc < 2) {
        cerr << "Usage:\n"
             << "  sim bimodal <M2> <tracefile>\n"
             << "  sim gshare <M1> <N> <tracefile>\n"
             << "  sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
        return 1;
    }

    // The first argument is the branch predictor type
    string predType = argv[1];

    // ------------------------------------------------------------------
    // Bimodal
    //   sim bimodal <M2> <tracefile>
    // ------------------------------------------------------------------
    if (predType == "bimodal") {
        if (argc < 4) {
            cerr << "Usage: sim bimodal <M2> <tracefile>\n";
            return 1;
        }
        int m = stoi(argv[2]);
        string filename = argv[3];

        // Build a command string to show in output
        string cmd = "./sim " + predType + " " + to_string(m) + " " + filename;

        // Create Bimodal Predictor
        BimodalPredictor predictor(m);

        // Open the trace
        ifstream traceFile("traces/" + filename);
        if (!traceFile.is_open()) {
            cerr << "Unable to open trace file\n";
            return 1;
        }

        // Read each line: <PC> <t/n>
        string line;
        while (getline(traceFile, line)) {
            stringstream ss(line);
            vector<string> address_and_label;
            string token;
            while (ss >> token) {
                address_and_label.push_back(token);
            }
            if (address_and_label.size() == 2) {
                predictor.predict(address_and_label[0], address_and_label[1]);
            }
        }
        traceFile.close();

        // Print results
        predictor.generate_val_traces(cmd);
    }

    // ------------------------------------------------------------------
    // GShare
    //   sim gshare <M1> <N> <tracefile>
    // ------------------------------------------------------------------
    else if (predType == "gshare") {
        if (argc < 5) {
            cerr << "Usage: sim gshare <M1> <N> <tracefile>\n";
            return 1;
        }
        int m = stoi(argv[2]);
        int n  = stoi(argv[3]);
        string filename = argv[4];

        // Build a command string to show in output
        string cmd = "./sim " + predType + " " + to_string(m) + " " + to_string(n) + " " + filename;

        // Create GShare Predictor
        GSharePredictor predictor(n, m);  // or GSharePredictor(M1, N) depending on your constructor ordering

        // Open the trace
        ifstream traceFile("traces/" + filename);
        if (!traceFile.is_open()) {
            cerr << "Unable to open trace file\n";
            return 1;
        }

        // Read each line: <PC> <t/n>
        string line;
        while (getline(traceFile, line)) {
            stringstream ss(line);
            vector<string> address_and_label;
            string token;
            while (ss >> token) {
                address_and_label.push_back(token);
            }
            if (address_and_label.size() == 2) {
                predictor.predict(address_and_label[0], address_and_label[1]);
            }
        }
        traceFile.close();

        // Print results
        predictor.generate_val_traces(cmd);
    }

    // ------------------------------------------------------------------
    // Hybrid
    //   sim hybrid <K> <M1> <N> <M2> <tracefile>
    // ------------------------------------------------------------------
    else if (predType == "hybrid") {
        if (argc < 7) {
            cerr << "Usage: sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
            return 1;
        }
        int K  = stoi(argv[2]);
        int M1 = stoi(argv[3]);
        int N  = stoi(argv[4]);
        int M2 = stoi(argv[5]);
        string filename = argv[6];

        // Build a command string to show in output
        string cmd = "./sim " + predType + " " +
                     to_string(K) + " " +
                     to_string(M1) + " " +
                     to_string(N) + " " +
                     to_string(M2) + " " + filename;

        // Create Hybrid Predictor
        HybridPredictor predictor(K, M1, N, M2);

        // Open the trace
        ifstream traceFile("traces/" + filename);
        if (!traceFile.is_open()) {
            cerr << "Unable to open trace file\n";
            return 1;
        }

        // Read each line: <PC> <t/n>
        string line;
        while (getline(traceFile, line)) {
            stringstream ss(line);
            vector<string> address_and_label;
            string token;
            while (ss >> token) {
                address_and_label.push_back(token);
            }
            if (address_and_label.size() == 2) {
                predictor.predict(address_and_label[0], address_and_label[1]);
            }
        }
        traceFile.close();

        // Print results
        predictor.generate_val_traces(cmd);
    }

    // ------------------------------------------------------------------
    // Unknown predictor type
    // ------------------------------------------------------------------
    else {
        cerr << "Unknown predictor type: " << predType << "\n";
        cerr << "Usage:\n"
             << "  sim bimodal <M2> <tracefile>\n"
             << "  sim gshare <M1> <N> <tracefile>\n"
             << "  sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
        return 1;
    }

    return 0;
}
