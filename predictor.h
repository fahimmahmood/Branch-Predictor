// predictor.h
#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <string>

class Predictor {
public:
    virtual ~Predictor() {}

    // Predict given a PC. Result could be "t" (taken) or "n" (not taken).
    virtual void predict(const std::string& pc_address, const std::string& label) = 0;

    // Optionally store actual result for training the predictor.
    virtual const std::string getLastPrediction() = 0;

    virtual void update_table(const int& index, const std::string& label, const std::string& prediction) = 0;
};

#endif // PREDICTOR_H