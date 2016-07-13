#ifndef OFXURG_H
#define OFXURG_H

#include "ofMain.h"

extern "C"{
#include "urg_sensor.h"
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

	void drawRadius();

	std::vector<Data> getData();
	std::vector<ofVec2f> getPoints(float minDistance=0.f);
	std::vector<Data> getDataRaw();

	void setROI(ofRectangle rect);
	void setROI(ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d);
	void setROI(ofPolyline poly);

	ofPolyline getROI();

	ofEvent<std::vector<Data>> onNewDataThread;

private:

	void setupInternal(std::string port);

	void update(ofEventArgs& args);

	void drawDataRadial(const std::vector<Data>& data, float scale=1.f);

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
};


#endif // OFXURG_H
