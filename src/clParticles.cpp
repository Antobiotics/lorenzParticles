#include "clParticles.h"

#define USE_OPENGL_CONTEXT
#define NUM_PARTICLES (1000 * 1)

//------------------------------------------------------------------------------
//																		  GLOBAL
//																	   VARIABLES
//------------------------------------------------------------------------------
ofColor white;
Particle particles[NUM_PARTICLES];
float2 particlesPos[NUM_PARTICLES];

//------------------------------------------------------------------------------
//																		 PRIVATE
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::setupPosition(int i) {
	float x = ofGetWidth() / 2;
	float y = ofGetHeight() / 2;
	particlesPos[i].set(x, y);
}

//------------------------------------------------------------------------------
void clParticles::setupParticles() {
	for(int i = 0; i < NUM_PARTICLES; i++) {
		Particle p = particles[i];
		p.vel.set(0, 0);
		p.mass = ofRandom(0.5, 1);
		p.theta = ofRandom(0, TWO_PI);
		p.u = ofRandom(-1, 1);
		setupPosition(i);
	}
}

//------------------------------------------------------------------------------
//																			  OF
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::setup() {
	// Initialise the window:
	white = *new ofColor(255, 255, 255);
	ofBackground(white);
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// Initialise Particle System:
	setupParticles();
}
//------------------------------------------------------------------------------

void clParticles::update(){

}

//------------------------------------------------------------------------------

void clParticles::draw(){

}

//------------------------------------------------------------------------------
//																	   CALLBACKS
//------------------------------------------------------------------------------
void clParticles::keyPressed(int key){

}

//------------------------------------------------------------------------------
void clParticles::keyReleased(int key){

}

//------------------------------------------------------------------------------
void clParticles::mouseMoved(int x, int y ){

}

//------------------------------------------------------------------------------
void clParticles::mouseDragged(int x, int y, int button){

}

//------------------------------------------------------------------------------
void clParticles::mousePressed(int x, int y, int button){

}

//------------------------------------------------------------------------------
void clParticles::mouseReleased(int x, int y, int button){

}

//------------------------------------------------------------------------------
void clParticles::windowResized(int w, int h){

}

//------------------------------------------------------------------------------
void clParticles::gotMessage(ofMessage msg){

}

//------------------------------------------------------------------------------

void clParticles::dragEvent(ofDragInfo dragInfo){ 

}
//------------------------------------------------------------------------------
