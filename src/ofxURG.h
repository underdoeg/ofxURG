#ifndef OFXURG_H
#define OFXURG_H

#include "ofMain.h"

extern "C"{
    #include "urg_sensor.h"
}

class ofxURG{
public:
    struct Data{
        float degrees;
        long distance;
    };

    ofxURG();

    void setup(std::string port="/dev/ttyACM0");
    void update();
    void drawRadius();

    std::vector<Data> data;

private:
    void readSensorCapabilities();
    void printLastError();
    bool checkError(int ret);

    std::vector<long> dataRaw;

    urg_t urg;

    long minDistance;
    long maxDistance;

    long lastTimeStamp;
};

#endif // OFXURG_H
