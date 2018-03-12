#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    urg.setup();

    // example: ROI with simple rectangle
    urg.setRoi(ofRectangle(-500, -2100, 1000, 2000));

    // example: ROI with irregular shape
    // ofPolyline line;
    // line.addVertex(-500, -2100);
    // line.addVertex(550, -2200);
    // line.addVertex(505, -200);
    // line.addVertex(-400, -300);
    // line.addVertex(-600, -1000);
    // line.close();
    // urg.setRoi(line);

    urg.start();
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
    urg.drawRadius();

    vector<ofVec2f> maskPoints = urg.getMaskPoints();
    urg.drawPoints(maskPoints, 150, ofColor(0,0,0,128), false);

    vector<ofVec2f> calibrationPoints = urg.getPoints();
    urg.drawPoints(calibrationPoints, 10, ofColor::azure, false);

    vector<ofVec2f> blobs = urg.getPoints(300);
    urg.drawPoints(blobs, 100);

    int numblobs = blobs.size();

    ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()) + "fps", 30, 30);
    ofDrawBitmapStringHighlight(ofToString(numblobs) + " blobs", 30, 50);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == ' ') {
        urg.calibrateMask(150); // set 15cm tolerance
    }
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
