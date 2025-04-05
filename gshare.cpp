#include<stdio.h>
#include<iostream>
#include<vector>
#include<string>
#include<cmath>
#include<fstream>
#include<sstream>
#include <iomanip>


using namespace std;

class GSharePredictor{
    private:
        string gshare_type;
        int prediction_threshold;
        int counter_max;
        int mispredictions;
        int num_predictions;
        string prediction;
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
    
    void predict(const string& pc_address, const string& label){
        this->num_predictions++;
        int pc_val = stoul(pc_address, nullptr, 16);
        pc_val = (pc_val >> 2) & this->table_size;

        int index = pc_val ^ (this->global_history_register_val & ((1 << this->global_history_register_bit_size) - 1));
        
        if(table[index] < this->prediction_threshold){
            this->prediction = "n";
        }
        else{
            this->prediction = "t";
        }

        if (this->prediction != label){
            this->mispredictions++;
        }
        update_table(index, label, this->prediction);
    }

    void update_table(const int& index, const string& label, const string& prediction){
        if(label =="t"){
            table[index] = min(this->counter_max, table[index]+1);
        }
        else{
            table[index] = max(0, table[index]-1);
        }
        if (this->gshare_type == "GShare"){
            update_global_history(label);
        }
        
    }

    void update_global_history(const string& label){
        this->global_history_register_val >>= 1;
        if (label == "t" && this->global_history_register_bit_size > 0) {
        this->global_history_register_val |= (1 << (this->global_history_register_bit_size - 1));
}
    }

    void generate_val_traces(const string& cmd){
        cout << "COMMAND\n";
        cout << cmd << "\n";
        cout << "OUTPUT\n";
        cout << "number of Predictions: " << this->num_predictions << "\n";
        cout << "number of Mispredictions: " << this->mispredictions << "\n";
        cout << "misprediction Rate: "<< std::fixed << std::setprecision(2)<< (100.0 * static_cast<double>(this->mispredictions) / this->num_predictions)<< "%" << "\n";cout << "FINAL GSHARE CONTENTS" << "\n";
        for(int i=0; i<this->table.size(); i++){
            cout << i << "\t" << table[i] << "\n";
        }
    }
};


int main(int argc, char* argv[]){
    
    string branch_pred_type=argv[1];
    
    int m = stoi(argv[2]);
    int n = stoi(argv[3]);

    string filename = argv[4];
    string cmd = "sim " + branch_pred_type + " " + argv[2] + " " + argv[3] + " " + filename ;
    
    
    ifstream traceFile;

    GSharePredictor predictor(n, m);
    string line;
    traceFile.open("traces/"+filename);
    
    
    if(traceFile.is_open()){
        while (getline(traceFile, line))
        {
            // predictor.predict();
            stringstream ss(line);
            string word;
            vector<string> address_and_label;

            while (ss >> word) { // Extract words separated by spaces
                address_and_label.push_back(word);
        
            }
            predictor.predict(address_and_label[0], address_and_label[1]);
            // cout << address_and_label[0] << " " << address_and_label[1] << "\n";
        }
        traceFile.close();
        predictor.generate_val_traces(cmd);
    }
    else{
        cout << "Unable to open trace file\n";
    }
    
}