#include "ofxURG.h"

extern "C"{
//#include "urg_utils.h"
}

ofxURG::ofxURG():
	bNewData(false),
	bSetup(false),
	drawZoom(1){
	ofAddListener(onNewDataThread, this, &ofxURG::newDataThread);

	commonPortNames = {"/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyACM2", "/dev/ttyACM3", "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyUSB3"};
	commonPortNamesIter = commonPortNames.begin();	
}

ofxURG::~ofxURG(){
	waitForThread(true);
	urg_disconnect(&urg);
}

void ofxURG::setup(string port){
	urg_initialize(&urg);
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

	ofLogNotice("ofxURG") << "open port " << port;

	if(!ofFile::doesFileExist(port, false) || checkError(urg_connect(&urg, port.c_str(), 115200))){
		ofLogError() << "Could not open " << port;
		setupInternal("");
		return;
	}

	ofLogNotice("ofxURG") << "connected to " << port;

	commonPortNamesIter = commonPortNames.begin();

	readSensorCapabilities();
	bSetup = true;
}

void ofxURG::start(){
	checkError(urg_laserOn(&urg));
	startThread();
}

void ofxURG::stop(){
	waitForThread(true);
	checkError(urg_laserOff(&urg));
}

bool ofxURG::isRunning(){
	return isThreadRunning();
}

void ofxURG::setAngleMinMax(float min, float max){
	//checkError(urg_(&urg, urg_deg2index(&urg, min), urg_deg2index(&urg, max), urg.scanning_skip_step));
}

void ofxURG::setStepSize(int step){
	int minStep, maxStep;
	checkError(urg_setSkipLines(&urg, step));
	//checkError(urg_set_scanning_parameter(&urg, minStep, maxStep, step));
}

int ofxURG::getStepSize(){
	return urg.skip_lines_;
}

void ofxURG::threadedFunction(){
	checkError(urg_setCaptureTimes(&urg, 0));

	while(isThreadRunning()){
		checkError(urg_requestData(&urg, URG_GD, URG_FIRST, URG_LAST));
		if(checkError(urg_receiveData(&urg, dataRaw.data(), dataRaw.size()))){
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
			dataThread[i].degrees = urg_index2deg(&urg, i*getStepSize())-90;
			dataThread[i].distance = dataRaw[i];
		}

		ofNotifyEvent(onNewDataThread, dataThread);
	}


	//checkError(urg_stop_measurement(&urg));
}

void ofxURG::readSensorCapabilities(){
	int dataSize = urg_dataMax(&urg);
	if(dataSize > 0)
		dataRaw.resize(dataSize);
	else{
		ofLogError("ofxURG") << "Max datasize wrong value: " << dataSize;
	}
	minDistance = urg_minDistance(&urg);
	maxDistance = urg_maxDistance(&urg);
	//urg_(&urg, &minDistance, &maxDistance);
	ofLogNotice("ofxURG") << "Data Size: " << dataSize;
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

void ofxURG::drawRadius(ofColor rawColor, ofColor filteredColor){
	auto filteredData = getData();

	ofPushMatrix();
	ofPushStyle();

	ofTranslate(ofGetWidth()*.5, ofGetHeight()*.5);

	ofScale(getDrawScale(), getDrawScale(), getDrawScale());

	ofSetLineWidth(2);

	ofSetColor(rawColor);
	drawDataRadial(getDataRaw());

	ofSetColor(filteredColor);
	drawDataRadial(filteredData);

	ofSetLineWidth(1);
	ofSetColor(0);
	roi.draw();

	ofFill();
	ofSetColor(0);
	ofDrawCircle(0, 0, 2);

	ofPopStyle();
	ofPopMatrix();
}

void ofxURG::drawPoints(std::vector<ofVec2f> points, float pointSize, ofColor pointColor, bool positionLabels) {
	ofPushMatrix();
	ofPushStyle();

	ofTranslate(ofGetWidth()*.5, ofGetHeight()*.5);

	ofScale(getDrawScale(), getDrawScale(), getDrawScale());

	ofSetColor(pointColor);
	for (auto p: points) {
        ofDrawCircle(p.x, p.y, pointSize);
		if (positionLabels) {
			ofDrawBitmapString(ofToString(p.x,0) + "," + ofToString(p.y,0), p.x + pointSize, p.y);
		}
    }
	ofPopStyle();
	ofPopMatrix();

}

void ofxURG::drawDataRadial(const std::vector<ofxURG::Data>& data){
	//ofBeginShape();
	for(auto& d: data){
		float x = cosf(ofDegToRad(d.degrees)) * d.distance;
		float y = sinf(ofDegToRad(d.degrees)) * d.distance;
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

std::vector<ofVec2f> ofxURG::getPoints(float pointSeparationDistance){
	std::vector<ofVec2f> points;
	for(auto& d: getData()){
		points.push_back(d.getPosition());
	}

	if(pointSeparationDistance == 0)
		return points;

	float minDistSquared = pointSeparationDistance*pointSeparationDistance;

	std::vector<std::vector<ofVec2f>> clusters;
	for(auto p: points){
		bool newCluster = true;
		std::vector<ofVec2f>* cluster = nullptr;
		for(auto& c: clusters){
			if(!newCluster)
				continue;
			for(auto cp: c){
				if(cp.squareDistance(p)<minDistSquared){
					//c.push_back(p);
					newCluster = false;
					cluster = &c;
					continue;
				}
			}
		}


		if(newCluster){
			clusters.push_back({p});
		}else{
			cluster->push_back(p);
		}
	}

	//get centers of all clusters
	std::vector<ofVec2f> ret;
	for(auto& c:clusters){
		ofVec2f center;
		for(auto& p: c){
			center+=p;
		}
		center /= float(c.size());
		ret.push_back(center);
	}

	return ret;
}

std::vector<ofxURG::Data> ofxURG::getDataRaw(){
	std::lock_guard<std::mutex> lock(mutex);
	return dataExchange;
}

void ofxURG::setRoi(ofRectangle rect){
	setRoi(ofPolyline::fromRectangle(rect));
}

void ofxURG::setRoi(ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d){
	ofPolyline poly;
	poly.addVertices({a, b, c, d});
	setRoi(poly);
}

void ofxURG::setRoi(std::vector<ofVec2f> points){
	ofPolyline poly;
	for(auto p: points){
		poly.addVertex(p);
	}
	setRoi(poly);
}

void ofxURG::setRoi(ofPolyline poly){
	poly.close();
	lock();
	roi = poly;
	unlock();
}

std::vector<ofVec2f> ofxURG::getRoiPoints(){
	std::vector<ofVec2f> pts;
	for(auto p: getRoi()){
		pts.push_back(p);
	}
	return pts;
}

ofPolyline ofxURG::getRoi(){
	std::lock_guard<std::mutex> lock(mutex);
	return roi;
}

float ofxURG::getDrawScale(){
	return float(ofGetHeight())/maxDistance*getDrawZoom();
}

float ofxURG::getDrawZoom() const{
	return drawZoom;
}

void ofxURG::setDrawZoom(float value){
	if(value < .001)
		value = .001;
	drawZoom = value;
}
