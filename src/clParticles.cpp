//----------------------
#include "clParticles.h"
//----------------------

#define USE_OPENGL_CONTEXT
#define NUM_PARTICLES (1000 * 100)

using namespace std;

#define kArg_particles   0
#define kArg_posBuffer   1
#define kArg_mousePos    2
#define kArg_dimensions  3
#define kArg_parameters  4
#define kArg_currentTime 5
#define kArg_dTime		 6

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
float2 initPos;
float4 parameters;
float2 mousePos;
float2 dimensions;
float currentTime;
float dTime;

int pointSize;

ofImage particuleTex;

//------------------------------------------------------------------------------
//																		   SETUP
//																	   FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void clParticles::setupWindow() {
	ofBackground(backgroundColor);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
	ofSetBackgroundAuto(true);
}

//------------------------------------------------------------------------------
void clParticles::setupParameters() {
	backgroundColor = *new ofColor(123, 12, 55);
	pointSize = 3;
	currentTime = 0;
	dTime = 0.01;
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
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, NULL);
	
	// Load Program:
	opencl.loadProgramFromFile("kernels/lorenz.cl");
	
	// Load the kernel:
	clLorenzKernel = opencl.loadKernel("updateParticle");
	
	clMemParticles.initBuffer(sizeof(Particle) * NUM_PARTICLES, CL_MEM_READ_WRITE, particles);
	clMemPosVBO.initFromGLObject(vbo[0]);

	// Bind variables to the kernel
	clLorenzKernel->setArg(kArg_particles, clMemParticles.getCLMem());
	clLorenzKernel->setArg(kArg_posBuffer, clMemPosVBO.getCLMem());
	clLorenzKernel->setArg(kArg_mousePos, mousePos);
	clLorenzKernel->setArg(kArg_dimensions, dimensions);
	clLorenzKernel->setArg(kArg_parameters, parameters);
	clLorenzKernel->setArg(kArg_currentTime, currentTime);
	clLorenzKernel->setArg(kArg_dTime, dTime);
}
//------------------------------------------------------------------------------
void clParticles::setupPosition(int i) {
	initPos.x = ofGetWidth() / 2;
	initPos.y = ofGetHeight() / 2;
	particlesPos[i] = initPos;
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
//																		  UPDATE
//																	   FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  																		DRAW
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::drawParticles() {
	//
	glPushMatrix();
		glColor3f(1.0f, 1.0f, 1.0f);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_BLEND_SRC_ALPHA, GL_ONE);
		glPointSize(pointSize);

	opencl.finish(); // block previously queued OpenCL commands.
	glEnableClientState(GL_VERTEX_ARRAY);
	
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
		glVertexPointer(2, GL_FLOAT, 0, NULL);
//	particuleTex.getTextureReference().bind();
		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
//	particuleTex.getTextureReference().unbind();
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, NULL);
	glPopMatrix();
	//
}

//------------------------------------------------------------------------------
void clParticles::drawInfos() {
	glColor3f(1.0f, 1.0f, 1.0f);
	string info = "fps: " + ofToString(ofGetFrameRate()) +
				  "\nnumber of particles: " + ofToString(NUM_PARTICLES);
	ofDrawBitmapString(info, 20, 20);
}

//------------------------------------------------------------------------------
//																			  OF
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::setup() {
	// Initialise Parameters and global variables:
	setupParameters();
	
	// Initialise the window:
	setupWindow();

	// Initialise OpenCL:
	setupOpenCL();
	
	// Initialise Particle System:
	setupParticles();
	
	ofDisableArbTex();
	particuleTex.loadImage("glitter.png");
}
//------------------------------------------------------------------------------

void clParticles::update() {
	// Update Arguments:
    mousePos.x   = ofGetMouseX();
	mousePos.y   = ofGetMouseY();
	dimensions.x = ofGetWidth ();
	dimensions.y = ofGetHeight();
	
	// Set Kernel Arguments:
	clLorenzKernel->setArg(kArg_mousePos, mousePos);
	clLorenzKernel->setArg(kArg_dimensions, dimensions);
	clLorenzKernel->setArg(kArg_dTime, dTime);
	clLorenzKernel->setArg(kArg_currentTime, currentTime);
	
    // Update the OpenCL kernel:
    clEnqueueAcquireGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0, NULL, NULL);
	clLorenzKernel->run1D(NUM_PARTICLES);
	clEnqueueReleaseGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0, NULL, NULL);
    
    // Update Global Variables:
    currentTime += dTime;
}

//------------------------------------------------------------------------------

void clParticles::draw() {
	drawParticles();
	drawInfos();
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
