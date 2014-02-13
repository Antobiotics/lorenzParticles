#pragma once

//--------------------
#include "ofMain.h"
#include "MSAOpenCL.h"
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
	
private:
	void setupParameters();
	void setupWindow();
	void setupOpenCL();
	void setupParticles();
	void setupPosition(int i);
	
	void drawParticles();
	void drawInfos();
};

//------------------------------------------------------------------------------