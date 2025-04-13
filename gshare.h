#ifndef GSHARE_H
#define GSHARE_H
#include<string>
#include<vector>

class GSharePredictor{
    private:
        std::string gshare_type;
        int prediction_threshold;
        int counter_max;
        int mispredictions;
        int num_predictions;
        std::string prediction;
        vector<int> table;

        int global_history_register_val;
        int global_history_register_bit_size;
        int pc_val;
        int pc_bit_size;
        int table_size;
        
    
    public:
        GSharePredictor(int n, int m):
            global_history_register_bit_size(n),
            pc_bit_size(m){
                this->gshare_type = (global_history_register_bit_size == 0) ? "Bimodal" : "GShare";
                this->prediction_threshold = 4;
                this->counter_max = 7;
                this->mispredictions = 0;
                this->num_predictions = 0;
                this->global_history_register_val = 0;
                this->table = vector<int>(1<<pc_bit_size, 4);
                this->table_size = (1<<pc_bit_size) - 1;
                this->prediction = "t";
            }
        void predict(const std::string& pc_address, const std::string& label);
        void update_table(const int& index, const std::string& label, const std::string& prediction);
        void update_global_history(const std::string& label);
        void generate_val_traces(const std::string& cmd);
        std::string getLastPrediction() const;
};

#endif