#pragma once

//--------------------
#include "ofMain.h"
#include "ofxUI.h"
#include "MSAOpenCL.h"
#include "ofxMSAfft.h"
#include "particle.h"
//--------------------

//------------------------------------------------------------------------------
class clParticles : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
	void exit();
	void guiEvent(ofxUIEventArgs &e);
	
private:
	void setupParameters();
	void setupFFT();
	void setupWindow();
	void setupGUI();
	void setupOpenCL();
	void setupOpenGL();
	void setupParticles();
	void setupPosition(int i);
	void setupNodes();
	
	void drawParticles();
	void drawNodes();
	void drawInfos();
	void drawGUI();
	void drawFBOs();
	void drawFFT();
};

//------------------------------------------------------------------------------