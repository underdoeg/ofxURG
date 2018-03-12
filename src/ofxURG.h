#ifndef OFXURG_H
#define OFXURG_H

#include "ofMain.h"

extern "C"{
#include "urg_ctrl.h"
}

class ofxURG: private ofThread{
public:
	class Data{
	public:
		ofVec2f getPosition(){
			float rad = ofDegToRad(degrees);
			return ofVec2f(cos(rad)*distance, sin(rad)*distance);
		}

		void setPosition(ofVec2f p){
			degrees = ofRadToDeg(atan2f(p.y, p.x));
			distance = long(p.length());
		}

		double degrees;
		long distance;
	};

	ofxURG();
	virtual ~ofxURG();

	void setup(std::string port="");

	void start();
	void stop();

	bool isRunning();

	void setAngleMinMax(float min, float max);
	void setStepSize(int step);

	int getStepSize();

	void drawRadius(ofColor rawColor = ofColor::darkGrey, ofColor filteredColor = ofColor::mediumBlue);
	void drawPoints(float pointSize=10.0f);

	std::vector<Data> getData();
	std::vector<ofVec2f> getPoints();
	std::vector<Data> getDataRaw();

	void setRoi(ofRectangle rect);
	void setRoi(ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d);
	void setRoi(std::vector<ofVec2f> points);
	void setRoi(ofPolyline poly);

	void setPointSeparation(float minDistance);

	std::vector<ofVec2f> getRoiPoints();
	ofPolyline getRoi();

	float getDrawScale();

	ofEvent<std::vector<Data>> onNewDataThread;

	float getDrawZoom() const;
	void setDrawZoom(float value);

private:

	void setupInternal(std::string port);

	void update(ofEventArgs& args);

	void drawDataRadial(const std::vector<Data>& data);

	void newDataThread(std::vector<Data>& data);
	void threadedFunction() override;

	void readSensorCapabilities();
	void printLastError();
	bool checkError(int ret);

	std::vector<long> dataRaw;
	std::vector<Data> dataThread;
	std::vector<Data> dataExchange;

	bool bNewData;

	urg_t urg;

	long minDistance;
	long maxDistance;

	long lastTimeStamp;

	bool bSetup;

	std::vector<std::string> commonPortNames;
	std::vector<std::string>::iterator commonPortNamesIter;

	ofPolyline roi;
	float pointSeparationDistance;

	float drawZoom;
};


#endif // OFXURG_H
