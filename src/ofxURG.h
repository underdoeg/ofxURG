#ifndef OFXURG_H
#define OFXURG_H

#include "ofMain.h"

extern "C"{
    #include "urg_sensor.h"
}

class ofxURG: private ofThread{
public:
    struct Data{
        float degrees;
        long distance;
    };

    ofxURG();
    ~ofxURG();

    void setup(std::string port="/dev/ttyACM0");

    void setAngleMinMax(float min, float max);
    void setStepSize(int step);

    int getStepSize();

    void drawRadius();

    std::vector<Data> data;

private:
    void update(ofEventArgs& args);
    void threadedFunction() override;

    void readSensorCapabilities();
    void printLastError();
    bool checkError(int ret);

    std::vector<long> dataRaw;
    std::vector<Data> dataThread;
    std::vector<Data> dataExchange;

    urg_t urg;

    long minDistance;
    long maxDistance;

    long lastTimeStamp;
};


#endif // OFXURG_H
