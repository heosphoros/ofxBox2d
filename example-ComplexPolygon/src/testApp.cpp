#include "testApp.h"

static bool shouldRemove(ofPtr<ofxBox2dBaseShape>shape) {
    return !ofRectangle(0, -400, ofGetWidth(), ofGetHeight()+400).inside(shape.get()->getPosition());
}

//--------------------------------------------------------------
void testApp::setup() {
	
    ofDisableAntiAliasing();
	ofBackgroundHex(0xfdefc2);
	ofSetLogLevel(OF_LOG_NOTICE);
	ofSetVerticalSync(true);
	
	// Box2d
	box2d.init();
	box2d.setGravity(0, 20);
	box2d.createGround();
	box2d.setFPS(30.0);
	
    breakupIntoTriangles = true;
	    
	// load the shape we saved...
	vector <ofPoint> pts = loadPoints("shape.dat");
    ofPtr<ofxBox2dPolygon> poly = ofPtr<ofxBox2dPolygon>(new ofxBox2dPolygon);
    poly.get()->addVertices(pts);
    poly.get()->setPhysics(1.0, 0.3, 0.3);
	poly.get()->create(box2d.getWorld());
	polyShapes.push_back(poly);
	
}

//--------------------------------------------------------------
vector <ofPoint> testApp::loadPoints(string file) {
    vector <ofPoint> pts;
    vector <string>  ptsStr = ofSplitString(ofBufferFromFile(file).getText(), ",");
    for (int i=0; i<ptsStr.size(); i+=2) {
        float x = ofToFloat(ptsStr[i]);
        float y = ofToFloat(ptsStr[i+1]);
        pts.push_back(ofPoint(x, y));
    }
	return pts;
}


//--------------------------------------------------------------
void testApp::update() {
	
	// add some circles every so often
	if((int)ofRandom(0, 10) == 0) {
        ofPtr<ofxBox2dCircle> circle = ofPtr<ofxBox2dCircle>(new ofxBox2dCircle);
        circle.get()->setPhysics(0.3, 0.5, 0.1);
		circle.get()->setup(box2d.getWorld(), (ofGetWidth()/2)+ofRandom(-20, 20), -20, ofRandom(10, 20));
		circles.push_back(circle);
	}
	
    // remove shapes offscreen
    ofRemove(circles, shouldRemove);
   // ofRemove(polyShapes, shouldRemove);
    
    
	box2d.update();	
}

//--------------------------------------------------------------
void testApp::draw() {
	
	
	// some circles :)
	for (int i=0; i<circles.size(); i++) {
		ofFill();
		ofSetHexColor(0xc0dd3b);
		circles[i].get()->draw();
	}
	
	ofSetHexColor(0x444342);
	ofFill();
	shape.draw();
	
	ofSetHexColor(0x444342);
	ofNoFill();
	for (int i=0; i<polyShapes.size(); i++) {
		polyShapes[i].get()->draw();
        
        ofCircle(polyShapes[i].get()->getPosition(), 3);
	}	
    
	
	// some debug information
	string info = "Draw a shape with the mouse\n";
	info += "Press 1 to add some circles\n";
	info += "Press c to clear everything\n";
	info += "Press t to break object up into triangles/convex poly: "+string(breakupIntoTriangles?"true":"false")+"\n";
    info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
	info += "Total Joints: "+ofToString(box2d.getJointCount())+"\n\n";
	info += "FPS: "+ofToString(ofGetFrameRate())+"\n";
    ofSetHexColor(0x444342);
	ofDrawBitmapString(info, 10, 15);
}


//--------------------------------------------------------------
void testApp::keyPressed(int key) {
	
	if(key == '1') {
        ofPtr<ofxBox2dCircle> circle = ofPtr<ofxBox2dCircle>(new ofxBox2dCircle);
        circle.get()->setPhysics(0.3, 0.5, 0.1);
		circle.get()->setup(box2d.getWorld(), mouseX, mouseY, ofRandom(10, 20));
		circles.push_back(circle);
	}
    
    if(key == 't') breakupIntoTriangles = !breakupIntoTriangles;
	
	if(key == 'c') {
		shape.clear();
        polyShapes.clear();
        circles.clear();
	}
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ) {
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {
	shape.addVertex(x, y);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {
	shape.clear();
	shape.addVertex(x, y);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {

    if(breakupIntoTriangles) {
        
        // This is the manual way to triangulate the shape
        // you can then add many little triangles
        
        // first simplify the shape
        shape.simplify();
        
        // save the outline of the shape
        ofPolyline outline = shape;
        
        // resample shape
        ofPolyline resampled = shape.getResampledBySpacing(25);
        
        // trangleate the shape, return am array of traingles
        vector <TriangleShape> tris = triangulatePolygonWithOutline(resampled, outline);
        
        // add some random points inside
        addRandomPointsInside(shape, 255);
        
        // now loop through all the trainles and make a box2d triangle
        for (int i=0; i<tris.size(); i++) {
            
            ofPtr<ofxBox2dPolygon> triangle = ofPtr<ofxBox2dPolygon>(new ofxBox2dPolygon);
            triangle.get()->addTriangle(tris[i].a, tris[i].b, tris[i].c);
            triangle.get()->setPhysics(1.0, 0.3, 0.3);
            triangle.get()->create(box2d.getWorld());

            polyShapes.push_back(triangle);
        }
      
    }
    else {
        
        // create a poly shape with the max verts allowed
        // and the get just the convex hull from the shape
        shape = shape.getResampledByCount(b2_maxPolygonVertices);
        shape = getConvexHull(shape);
        
        ofPtr<ofxBox2dPolygon> poly = ofPtr<ofxBox2dPolygon>(new ofxBox2dPolygon);
        poly.get()->addVertices(shape.getVertices());
        poly.get()->setPhysics(1.0, 0.3, 0.3);
        poly.get()->create(box2d.getWorld());
        polyShapes.push_back(poly);
        
        
    }
    
    // done with shape clear it now
    shape.clear();
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h) {
}

