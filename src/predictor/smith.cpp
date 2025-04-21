class NBitSmithPredictor{
    private:
        int counter;
        int maximum;
        int threshold;
        const int initialValue[7] = {0, 1, 2, 4, 8, 16, 32};
    public:
        // constructor
        NBitSmithPredictor(int size) {
            this->maximum = (1 << size) - 1; // set the saturation value to n-1
            this->threshold = (1 << size) / 2; // set the prediction threshold value to n/2
            this->counter = (size < 7) ? initialValue[size] : 0; // initialize the counter to a constant corresponding to the predictor size, or 0 if greater
        }

        // predict
        //      Make a prediction (t/n) based on the current counter state
        const std::string getLastPrediction() {
            if (this->counter >= this->threshold) {
                // predict taken
                return "t";
            }
            else {
                // predict not taken
                return "n";
            }
        }

        // update
        //      Update the counter state given an actual branch result
        void predict(const std::string& pc_address, const std::string& label) {
            if (label == "t") {
                // increment counter if branch was taken and counter isn't saturated
                if (this->counter < this->maximum) {
                    this->counter++;
                }
            }
            else {
                // decrement counter if branch not taken and counter isn't desaturated
                if (this->counter > 0) {
                    this->counter--;
                }
            }
        }

        // dumpState
        //      Outputs the current state of the counter
        int dumpState() {
            return this->counter;
        }
};