#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    urg.setup();

    // example: ROI with simple rectangle
    // urg.setRoi(ofRectangle(-500, -2100, 1000, 2000));

    // example: ROI with irregular shape
    ofPolyline line;
    line.addVertex(-500, -2100);
    line.addVertex(550, -2200);
    line.addVertex(505, -200);
    line.addVertex(-400, -300);
    line.addVertex(-600, -1000);
    line.close();
    urg.setRoi(line);

    urg.start();
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
    urg.drawRadius();

    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    unsigned int MIN_DIST_SEPARATION = 300;
    vector<ofVec2f> points = urg.getPoints(MIN_DIST_SEPARATION);
    float pointRadius = 10.0;

    int numPoints = 0;

    for (auto p: points) {
        ofSetColor(255,0,0);
        float mapX = ofMap(p.x, -5600, 5600, -ofGetWidth()/2, ofGetWidth()/2);
        float mapY = ofMap(p.y, -5600, 5600, -ofGetWidth()/2, ofGetWidth()/2);
        ofEllipse(mapX, mapY, pointRadius, pointRadius);
        ofDrawBitmapString(ofToString(p.x,0) + "," + ofToString(p.y,0), mapX + pointRadius, mapY);
        numPoints++;
    }
    ofPopMatrix();

    ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()) + "fps", 30, 30);
    ofDrawBitmapStringHighlight(ofToString(numPoints) + " blobs", 30, 50);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
	urg.setDrawZoom(urg.getDrawZoom()+scrollY*.05);
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
