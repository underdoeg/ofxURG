#include "ofxURG.h"

extern "C"{
#include "urg_utils.h"
}

ofxURG::ofxURG(){

}

void ofxURG::setup(string port){
    checkError(urg_open(&urg, URG_SERIAL, port.c_str(), 115200));
    readSensorCapabilities();
}

void ofxURG::update(){
    urg_start_measurement(&urg, URG_DISTANCE, 0, 0);
    checkError(urg_get_distance(&urg, dataRaw.data(), &lastTimeStamp));

    for(int i=0; i<dataRaw.size(); i++){
        data[i].degrees = urg_step2deg(&urg, i);
        data[i].distance = dataRaw[i];
    }
}

void ofxURG::drawRadius(){
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
