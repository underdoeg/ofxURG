#include "ofxURG.h"

extern "C"{
#include "urg_utils.h"
}

ofxURG::ofxURG():
	bNewData(false),
	bSetup(false){
	ofAddListener(onNewDataThread, this, &ofxURG::newDataThread);

	commonPortNames = {"/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyACM2", "/dev/ttyACM3", "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyUSB3"};
	commonPortNamesIter = commonPortNames.begin();
}

ofxURG::~ofxURG(){
	waitForThread(true);
}

void ofxURG::setup(string port){
	commonPortNamesIter = commonPortNames.begin();
	setupInternal(port);
}

void ofxURG::setupInternal(string port){
	if(ofTrim(port) == ""){
		if(commonPortNamesIter == commonPortNames.end())
			return;
		port = *commonPortNamesIter;
		commonPortNamesIter++;
	}

	if(checkError(urg_open(&urg, URG_SERIAL, port.c_str(), 115200))){
		ofLogError() << "Could not open " << port;
		setupInternal("");
		return;
	}

	commonPortNamesIter = commonPortNames.begin();

	readSensorCapabilities();
	bSetup = true;
}

void ofxURG::start(){
	checkError(urg_laser_on(&urg));
	startThread();
}

void ofxURG::stop(){
	waitForThread(true);
	checkError(urg_laser_off(&urg));
}

bool ofxURG::isRunning(){
	return isThreadRunning();
}

void ofxURG::setAngleMinMax(float min, float max){
	checkError(urg_set_scanning_parameter(&urg, urg_deg2step(&urg, min), urg_deg2step(&urg, max), urg.scanning_skip_step));
}

void ofxURG::setStepSize(int step){
	int minStep, maxStep;
	urg_step_min_max(&urg, &minStep, &maxStep);
	checkError(urg_set_scanning_parameter(&urg, minStep, maxStep, step));
}

int ofxURG::getStepSize(){
	return urg.scanning_skip_step;
}


void ofxURG::threadedFunction(){
	urg_start_measurement(&urg, URG_DISTANCE, URG_SCAN_INFINITY, 0);

	while(isThreadRunning()){
		if(checkError(urg_get_distance(&urg, dataRaw.data(), &lastTimeStamp))){
			ofLogError("ofxURG") << "Stopping Thread";
			stopThread();
			continue;
		}

		size_t numSteps = size_t(floor(dataRaw.size() / float(getStepSize())));
		dataThread.resize(numSteps);

		if(numSteps == 0){
			continue;
		}

		for(size_t i=0; i<numSteps; i++){
			dataThread[i].degrees = urg_step2deg(&urg, i*getStepSize());
			dataThread[i].distance = dataRaw[i];


		}

		ofNotifyEvent(onNewDataThread, dataThread);
	}

	urg_stop_measurement(&urg);
}

void ofxURG::readSensorCapabilities(){
	int dataSize = urg_max_data_size(&urg);
	if(dataSize > 0)
		dataRaw.resize(dataSize);
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
	auto data = getData();

	ofPushMatrix();
	ofPushStyle();

	ofTranslate(ofGetWidth()*.5, ofGetHeight()*.5);

	float scale = (ofGetHeight()*.65)/maxDistance;

	ofScale(scale);

	ofSetLineWidth(2);

	ofSetColor(180);
	drawDataRadial(getDataRaw());

	ofSetColor(60);
	drawDataRadial(data);

	ofSetLineWidth(1);
	ofSetColor(0);
	roi.draw();

	ofFill();
	ofSetColor(0);
	ofDrawCircle(0, 0, 2);

	ofPopStyle();
	ofPopMatrix();
}

void ofxURG::drawDataRadial(const std::vector<ofxURG::Data>& data, float scale){
	//ofBeginShape();
	for(auto& d: data){
		float x = cosf(ofDegToRad(d.degrees)) * d.distance * scale;
		float y = sinf(ofDegToRad(d.degrees)) * d.distance * scale;
		//ofVertex(x, y);
		ofDrawLine(0, 0, x, y);
	}
	//ofEndShape();
}

void ofxURG::newDataThread(std::vector<ofxURG::Data>& data){
	std::lock_guard<std::mutex> lock(mutex);
	dataExchange = data;
}

std::vector<ofxURG::Data> ofxURG::getData(){
	lock();
	std::vector<Data> data = dataExchange;
	unlock();

	std::vector<Data> filtered;

	if(roi.size() > 0){
		for(auto& d: data){
			ofVec2f p = d.getPosition();
			/*
			if(!roi.inside(p)){
				//dataThread[i].setPosition(roiThread.getClosestPoint(p));
				d.distance = 0;
			}
			*/
			if(roi.inside(p)){
				filtered.push_back(d);
			}
		}
	}else{
		filtered = data;
	}

	return filtered;
}

std::vector<ofxURG::Data> ofxURG::getDataRaw(){
	std::lock_guard<std::mutex> lock(mutex);
	return dataExchange;
}

void ofxURG::setROI(ofRectangle rect){
	setROI(ofPolyline::fromRectangle(rect));
}

void ofxURG::setROI(ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d){
	ofPolyline poly;
	poly.addVertices({a, b, c, d});
	setROI(poly);
}

void ofxURG::setROI(ofPolyline poly){
	lock();
	roi = poly;
	unlock();
}

ofPolyline ofxURG::getROI(){
	std::lock_guard<std::mutex> lock(mutex);
	return roi;
}

