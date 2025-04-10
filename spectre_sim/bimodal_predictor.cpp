#include "bimodal_predictor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace std;

BimodalPredictor::BimodalPredictor(int m)
    : pc_bit_size(m), prediction("t") {
    prediction_threshold = 4;
    counter_max = 7;
    mispredictions = 0;
    num_predictions = 0;
    table_size = (1 << pc_bit_size) - 1;
    table = vector<int>(1 << pc_bit_size, 4);
}

void BimodalPredictor::predict(const string& pc_address, const string& label) {
    num_predictions++;
    int pc_val = stoul(pc_address, nullptr, 16);
    pc_val = (pc_val >> 2) & table_size;

    int index = pc_val;

    prediction = (table[index] < prediction_threshold) ? "n" : "t";

    if (prediction != label) {
        mispredictions++;
    }

    update_table(index, label, prediction);
}

void BimodalPredictor::update_table(const int& index, const string& label, const string& prediction) {
    if (label == "t") {
        table[index] = min(counter_max, table[index] + 1);
    } else {
        table[index] = max(0, table[index] - 1);
    }
}

void BimodalPredictor::generate_val_traces(const string& cmd) {
    cout << "COMMAND\n" << cmd << "\n";
    cout << "OUTPUT\n";
    cout << "number of Predictions: " << num_predictions << "\n";
    cout << "number of Mispredictions: " << mispredictions << "\n";
    cout << "misprediction Rate: " << fixed << setprecision(2)
         << (100.0 * mispredictions / num_predictions) << "%\n";
    cout << "FINAL BIMODAL CONTENTS\n";
    for (int i = 0; i < table.size(); i++) {
        cout << i << "\t" << table[i] << "\n";
    }
}

string BimodalPredictor::getLastPrediction() const {
    return prediction;
}
