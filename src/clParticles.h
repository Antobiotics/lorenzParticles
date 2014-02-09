#pragma once

//-----------------
#include "ofMain.h"
#include "MSAOpenCL.h"
//-----------------

//------------------------------------------------------------------------------
typedef struct{
	float2 vel;
	float mass;
	float dummy;
    float theta;
    float vTheta;
    float u;
    float vU;
} Particle;

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
	void setupParticles();
	void setupPosition(int i);
};

//------------------------------------------------------------------------------