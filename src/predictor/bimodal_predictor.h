#ifndef BIMODAL_PREDICTOR_H
#define BIMODAL_PREDICTOR_H

#include <string>
#include <vector>

class BimodalPredictor {
private:
    int prediction_threshold;
    int counter_max;
    int mispredictions;
    int num_predictions;
    std::string prediction;
    std::vector<int> table;
    int pc_bit_size;
    int table_size;

public:
    BimodalPredictor(int m);
    void predict(const std::string& pc_address, const std::string& label);
    void update_table(const int& index, const std::string& label, const std::string& prediction);
    void generate_val_traces(const std::string& cmd);
    std::string getLastPrediction() const;
};

#endif