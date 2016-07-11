#include "ofxURG.h"

extern "C"{
#include "urg_utils.h"
}

ofxURG::ofxURG(){

}

ofxURG::~ofxURG(){
    waitForThread(true);
}

void ofxURG::setup(string port){
    checkError(urg_open(&urg, URG_SERIAL, port.c_str(), 115200));
    readSensorCapabilities();
    ofAddListener(ofEvents().update, this, &ofxURG::update);
    startThread();
}

void ofxURG::setAngleMinMax(float min, float max){
    checkError(urg_set_scanning_parameter(&urg, urg_deg2step(&urg, min), urg_deg2step(&urg, max), urg.scanning_skip_step));
}

void ofxURG::setStepSize(int step){
    checkError(urg_set_scanning_parameter(&urg, urg.scanning_first_step, urg.scanning_last_step, step));
}

int ofxURG::getStepSize(){
    return urg.scanning_skip_step;
}

void ofxURG::update(ofEventArgs& args){
    lock();
    data = dataExchange;
    unlock();
}

void ofxURG::threadedFunction(){
    urg_start_measurement(&urg, URG_DISTANCE, URG_SCAN_INFINITY, 0);

    while(isThreadRunning()){
        checkError(urg_get_distance(&urg, dataRaw.data(), &lastTimeStamp));

        size_t numSteps = floor(dataRaw.size() / float(getStepSize()));
        dataThread.resize(numSteps);

        if(numSteps == 0){
            continue;
        }

        for(int i=0; i<numSteps; i++){
            dataThread[i].degrees = urg_step2deg(&urg, i*getStepSize());
            dataThread[i].distance = dataRaw[i];
        }

        lock();
        dataExchange = dataThread;
        unlock();
    }

    urg_stop_measurement(&urg);
}

void ofxURG::readSensorCapabilities(){
    dataRaw.resize(urg_max_data_size(&urg));
    data.resize(dataRaw.size());
    urg_distance_min_max(&urg, &minDistance, &maxDistance);
}

void ofxURG::printLastError(){
    ofLogError("ofxURG") << urg_error(&urg);
}

bool ofxURG::checkError(int ret){
    bool error = ret<0;
    if(error)
        printLastError();
    return error;
}

void ofxURG::drawRadius(){
    if(data.size() == 0)
        return;

    ofPushMatrix();
    ofPushStyle();

    ofTranslate(ofGetWidth()*.5, ofGetHeight()*.5);

    float scale = (ofGetHeight()*.5)/maxDistance;

    ofSetColor(80);
    ofBeginShape();
    for(auto& d: data){
        float x = cosf(ofDegToRad(d.degrees)) * d.distance * scale;
        float y = sinf(ofDegToRad(d.degrees)) * d.distance * scale;
        ofVertex(x, y);
    }
    ofEndShape();

    ofSetColor(0);
    ofDrawCircle(0, 0, 10);

    ofPopStyle();
    ofPopMatrix();
}
