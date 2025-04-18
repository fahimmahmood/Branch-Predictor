#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include "smith.cpp"
#include "bimodal.cpp"  // or place the bimodal class definition here
#include "gshare.cpp"   // or place the gshare class definition here
#include "hybrid.cpp"   // or place the hybrid class definition here

using namespace std;

int main(int argc, char* argv[])
{
    // Check if at least the predictor type is given
    if (argc < 2) {
        cerr << "Usage:\n"
             << "  sim smith <size> <tracefile>\n"
             << "  sim bimodal <M2> <tracefile>\n"
             << "  sim gshare <M1> <N> <tracefile>\n"
             << "  sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
        return 1;
    }

    // The first argument is the branch predictor type
    string predType = argv[1];

    // ------------------------------------------------------------------
    // N-bit Smith
    //   sim smith <size> <tracefile>
    // ------------------------------------------------------------------
    if (predType == "smith") {
        // Check the argument structure
        if (argc != 4) {
            cerr << "ERROR: bad arguments: ./sim smith <size (#bits)> <trace file>" << endl;
            return 1;
        }

        int size = atoi(argv[2]);
        string traceFile = argv[3];

        // Print benchmark input settings
        cout << "COMMAND\n./sim smith " << argv[2] << " " << argv[3] << endl;
        
        // Open the trace file
        ifstream file("traces/" + traceFile);
        if (!file) {
            cerr << "ERROR: couldn't open trace file" << endl;
            return 1;
        }

        // Instantiate the predictor and performance metrics
        NBitSmithPredictor smith(size);
        int predictions = 0;
        int mispredictions = 0;

        // Read from the trace file
        string trace;
        char predicted;
        while(getline(file, trace)) {
            // Isolate the address and result
            istringstream iss(trace);
            string address, result;
            iss >> address >> result;

            // Make a prediction
            predicted = smith.predict();

            // Update the predictor
            smith.update(result[0]);

            // Evaluate our preformance metrics
            predictions++;
            if (predicted != result[0]) {
                mispredictions++;
            }
        }

        file.close();

        // Print benchmark results
        cout << fixed << setprecision(2)
             << "OUTPUT\nnumber of predictions:\t\t" << predictions << endl
             << "number of mispredictions:\t" << mispredictions << endl
             << "misprediction rate:\t\t" << ((mispredictions * 100.0f) / predictions) << "%\n"
             << "FINAL COUNTER CONTENT:\t\t" << smith.dumpState() << endl;
    }

    // ------------------------------------------------------------------
    // Bimodal
    //   sim bimodal <M2> <tracefile>
    // ------------------------------------------------------------------
    else if (predType == "bimodal") {
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
             << "  sim smith <size> <tracefile>\n"
             << "  sim bimodal <M2> <tracefile>\n"
             << "  sim gshare <M1> <N> <tracefile>\n"
             << "  sim hybrid <K> <M1> <N> <M2> <tracefile>\n";
        return 1;
    }

    return 0;
}
