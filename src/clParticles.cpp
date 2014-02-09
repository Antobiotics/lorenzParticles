#include "clParticles.h"

#define USE_OPENGL_CONTEXT
#define NUM_PARTICLES (1000 * 100)

using namespace std;

//------------------------------------------------------------------------------
//																		  GLOBAL
//																	   VARIABLES
//------------------------------------------------------------------------------
msa::OpenCL opencl;

Particle particles[NUM_PARTICLES];
msa::OpenCLBuffer clMemParticles; // Stores particles.
float2 particlesPos[NUM_PARTICLES];
msa::OpenCLBuffer clMemPosVBO; // Stores particlesPos.
msa::OpenCLKernel *clLorenzKernel;

GLuint vbo[1];

ofColor backgroundColor;
float timeStep;
float dt;
float2 mousePos;
float2 dimensions;
float2 initPos;
int pointSize;

float tx, ty; // Translation values.

//------------------------------------------------------------------------------
//																		 PRIVATE
//																	   FUNCTIONS
//------------------------------------------------------------------------------
float2 clParticles::translateVector(float2 vec) {
	float2 translated;
	translated.x = vec.x - tx;
	translated.y = vec.y - ty;
	return translated;
}

//------------------------------------------------------------------------------
void clParticles::setupOpenCL() {
	opencl.setupFromOpenGL();
	
	// Bind buffer to the particle position.
	glGenBuffersARB(1, vbo); // Create a new buffer object.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]); // Bind the buffer object.
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,
					sizeof(float2) * NUM_PARTICLES,
					particlesPos,
					GL_STREAM_COPY_ARB); // Copy data to the buffer object.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	// Load Program:
	opencl.loadProgramFromFile("kernels/lorenz.cl");
	
	// Load the kernel:
	clLorenzKernel = opencl.loadKernel("updateParticle");
	
	clMemParticles.initBuffer(sizeof(Particle) * NUM_PARTICLES, CL_MEM_READ_WRITE, particles);
	clMemPosVBO.initFromGLObject(vbo[0]);

	// Bind variables to the kernel
	clLorenzKernel->setArg(0, clMemParticles.getCLMem());
	clLorenzKernel->setArg(1, clMemPosVBO.getCLMem());
	clLorenzKernel->setArg(2, mousePos);
	clLorenzKernel->setArg(3, dimensions);
}
//------------------------------------------------------------------------------
void clParticles::setupPosition(int i) {
	initPos.x = ofGetWidth() / 2;
	initPos.y = ofGetHeight() / 2;
	particlesPos[i].set(initPos);
}

//------------------------------------------------------------------------------
void clParticles::setupParticles() {
	for(int i = 0; i < NUM_PARTICLES; i++) {
		Particle *p = &particles[i];
		p->vel.set(ofRandom(-1, 1), ofRandom(-1, 1));
		p->mass = ofRandom(0.5, 1);
		p->theta = ofRandom(0, TWO_PI);
		p->u = ofRandom(-1, 1);
		setupPosition(i);
	}
}

//------------------------------------------------------------------------------
//																			  OF
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::setup() {
	// Initialise the window:
	backgroundColor = *new ofColor(123, 12, 55);
	pointSize = 5;
	tx = ofGetWidth() / 2;
	ty = ofGetHeight() / 2;
	timeStep = 0;
	ofBackground(backgroundColor);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	ofSetBackgroundAuto(true);
	
	// Initialise OpenCL:
	setupOpenCL();
	
	// Initialise Particle System:
	setupParticles();
	
	glPointSize(pointSize);
}
//------------------------------------------------------------------------------

void clParticles::update() {
	// Update Arguments:
    mousePos.x   = ofGetMouseX();
	mousePos.y   = ofGetMouseY();
	dimensions.x = ofGetWidth ();
	dimensions.y = ofGetHeight();
	
	// Set Kernel Arguments:
	clLorenzKernel->setArg(2, mousePos);
	clLorenzKernel->setArg(3, dimensions);
    
    // Update the OpenCL kernel:
    clEnqueueAcquireGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0,0,0);
	clLorenzKernel->run1D(NUM_PARTICLES);
	clEnqueueReleaseGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0,0,0);
    
    // Update Global Variables:
    timeStep += dt;
}

//------------------------------------------------------------------------------

void clParticles::draw() {
	
	glColor3f(1.0f, 1.0f, 1.0f);
	
	//
	glPushMatrix();
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
		opencl.finish(); // block previously queued OpenCL commands.
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, 0);
		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glPopMatrix();
	//
		
	glColor3f(1.0f, 1.0f, 1.0f);
	string info = "fps: " + ofToString(ofGetFrameRate()) + "\nnumber of particles: " + ofToString(NUM_PARTICLES);
	ofDrawBitmapString(info, 20, 20);
}

//------------------------------------------------------------------------------
//																	   CALLBACKS
//------------------------------------------------------------------------------
void clParticles::keyPressed(int key) {

}

//------------------------------------------------------------------------------
void clParticles::keyReleased(int key) {

}

//------------------------------------------------------------------------------
void clParticles::mouseMoved(int x, int y ) {

}

//------------------------------------------------------------------------------
void clParticles::mouseDragged(int x, int y, int button) {

}

//------------------------------------------------------------------------------
void clParticles::mousePressed(int x, int y, int button) {

}

//------------------------------------------------------------------------------
void clParticles::mouseReleased(int x, int y, int button) {

}

//------------------------------------------------------------------------------
void clParticles::windowResized(int w, int h) {

}

//------------------------------------------------------------------------------
void clParticles::gotMessage(ofMessage msg) {

}

//------------------------------------------------------------------------------

void clParticles::dragEvent(ofDragInfo dragInfo) {

}
//------------------------------------------------------------------------------
