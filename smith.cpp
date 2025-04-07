class NBitSmithPredictor {
    private:
        int counter;
        int maximum;
        int threshold;
        const int initialValue[5] = {0, 1, 2, 4, 8};
    public:
        // constructor
        NBitSmithPredictor(int size) {
            this->maximum = (1 << size) - 1; // set the saturation value to n-1
            this->threshold = (1 << size) / 2; // set the prediction threshold value to n/2
            this->counter = (size < 5) ? initialValue[size] : 0; // initialize the counter to a constant corresponding to the predictor size, or 0 if greater
        }

        // predict
        //      Make a prediction (t/n) based on the current counter state
        char predict() {
            if (this->counter >= this->threshold) {
                // predict taken
                return 't';
            }
            else {
                // predict not taken
                return 'n';
            }
        }

        // update
        //      Update the counter state given an actual branch result
        void update(char result) {
            if (result == 't') {
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