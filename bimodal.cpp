#include<stdio.h>
#include<iostream>
#include<vector>
#include<string>
#include<cmath>
#include<fstream>
#include<sstream>
#include <iomanip>


using namespace std;

class BimodalPredictor{
    private:
        int prediction_threshold;
        int counter_max;
        int mispredictions;
        int num_predictions;
        string prediction;
        vector<int> table;
        int pc_val;
        int pc_bit_size;
        int table_size;
        
    
    public:
        BimodalPredictor(int m):
            pc_bit_size(m){
                this->prediction_threshold = 4;
                this->counter_max = 7;
                this->mispredictions = 0;
                this->num_predictions = 0;
                this->table = vector<int>(1<<pc_bit_size, 4);
                this->table_size = (1<<pc_bit_size) - 1;
                this->prediction = "t";
            }
    
    void predict(const string& pc_address, const string& label){
        this->num_predictions++;
        int pc_val = stoul(pc_address, nullptr, 16);
        pc_val = (pc_val >> 2) & this->table_size;

        int index = pc_val; 
        
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
    }

    void generate_val_traces(const string& cmd){
        cout << "COMMAND\n";
        cout << cmd << "\n";
        cout << "OUTPUT\n";
        cout << "number of Predictions: " << this->num_predictions << "\n";
        cout << "number of Mispredictions: " << this->mispredictions << "\n";
        cout << "misprediction Rate: "<< std::fixed << std::setprecision(2)<< (100.0 * static_cast<double>(this->mispredictions) / this->num_predictions)<< "%" << "\n";cout << "FINAL BIMODAL CONTENTS" << "\n";
        for(int i=0; i<this->table.size(); i++){
            cout << i << "\t" << table[i] << "\n";
        }
    }
};


int main(int argc, char* argv[]){
    
    string branch_pred_type=argv[1];
    int m = stoi(argv[2]);

    string filename = argv[3];
    string cmd = "sim " + branch_pred_type + " " + argv[2] + " " + filename ;
    
    
    ifstream traceFile;

    BimodalPredictor predictor(m);
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
                }
        traceFile.close();
        predictor.generate_val_traces(cmd);
    }
    else{
        cout << "Unable to open trace file\n";
    }
    
}