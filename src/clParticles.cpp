//----------------------
#include "clParticles.h"
//----------------------

#define MAX_NUM_PARTICLES ( 1024 * 512 )
#define MAX_NUM_NODES 8


#define kArg_particles    0
#define kArg_nodes        1
#define kArg_posBuffer    2
#define kArg_colBuffer    3
#define kArg_mode 		  4
#define kArg_origin       5
#define kArg_color        6
#define kArg_mousePos     7
#define kArg_dimensions   8
#define kArg_prevAvgPower 9
#define kArg_fftPower     10
#define kArg_numNodes     11

#define kModeAudioReact 0
#define kModeExplode 1

#define FBO_CLEAR ofClear(255, 255, 255, 0)

using namespace std;
//------------------------------------------------------------------------------
//																		  GLOBAL
//																	   VARIABLES
//------------------------------------------------------------------------------
msa::OpenCL opencl;

Particle particles[MAX_NUM_PARTICLES];
msa::OpenCLBuffer clMemParticles; // Stores particles.
float2 positionBuffer[MAX_NUM_PARTICLES];
msa::OpenCLBuffer clMemPosVBO;    // Stores particlesPos.
float4 colorBuffer[MAX_NUM_PARTICLES];
msa::OpenCLBuffer clMemColVBO;    // Stores colors.
msa::OpenCLKernel *clKernel;

Node nodes[MAX_NUM_NODES];
msa::OpenCLBuffer clMemNodes;

GLuint vbo[2];

ofColor backgroundColor;
float4 color;
float4 parameters;
float2 mousePos;
float2 dimensions;
float currentTime;
float dTime;

unsigned int currentMode;
unsigned int numNodes;
unsigned int numParticles;
unsigned int pointSize;

bool  bDoVSync;
float radius;

float kickValue, snareValue, hihatValue;
float prevKickValue;
float currentMagnitude;
float previousMagnitude;
float2 initPos;

#ifdef FBOS
	ofFbo fboBlur;
	ofFbo fboPrev;
	ofFbo fboParticles;
#endif

string textureName = "images/bhole.png";


// Shaders:
ofShader shader;
ofImage particuleTex;

// GUI:
ofxUICanvas *gui;

// Beat Tracker:
ofxBeat beatTracker;
//------------------------------------------------------------------------------
//																		   SETUP
//																	   FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void clParticles::setupWindow() {
	ofBackground(backgroundColor);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
}

//------------------------------------------------------------------------------
void clParticles::setupGUI() {
	gui = new ofxUICanvas();
	gui->init(0, ofGetHeight()/8, ofGetWidth()/6, ofGetHeight());
//	gui->addSlider("Blur Amount", 0, 50, blurAmount);
//	gui->addSlider("Fade Speed", 0, 1, fadeSpeed);
	
	ofAddListener(gui->newGUIEvent, this, &clParticles::guiEvent);
	gui->autoSizeToFitWidgets();
	gui->loadSettings("UISettings.xml");
}
//------------------------------------------------------------------------------
void clParticles::setupParameters() {
	// Colors:
	backgroundColor = *new ofColor(255, 255, 255);
	color.x = 1.0f;
	color.y = 0.74f;
	color.z = 0.33f;
	color.w = 1.0f;
	
	numParticles = MAX_NUM_PARTICLES;
	
	// Kernel Parameters:
	prevKickValue = 0.1f;
	previousMagnitude = 0.0f;
	numNodes = MAX_NUM_NODES;
	// Time variables:
	currentTime = 0;
	dTime = 0.01f;
	
	// Dimensions:
	pointSize = 1;
	radius = 50;
	
	// Window:
	bDoVSync = true;
	
	// Modes:
	currentMode = kModeAudioReact;
}
//------------------------------------------------------------------------------
void clParticles::setupBeatTracker() {
	ofSoundStreamSetup(0, 1, this, 44100, beatTracker.getBufferSize(), 4);
}
//------------------------------------------------------------------------------
void clParticles::setupOpenCL() {
	opencl.setupFromOpenGL();
	
	// Bind buffer to the particle position.
	glGenBuffersARB(2, vbo); // Create two new buffer objects.
	{
		// The Particle Position Buffer:
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]); // Bind the buffer object.
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,
						sizeof(float2) * MAX_NUM_PARTICLES,
						positionBuffer,
						GL_STREAM_COPY_ARB); // Copy data to the buffer object.
		glVertexPointer(2, GL_FLOAT, 0, 0);
		
		// The Color Buffer:
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[1]);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,
						sizeof(float4) * MAX_NUM_PARTICLES,
						colorBuffer,
						GL_STREAM_COPY_ARB);
		glColorPointer(4, GL_FLOAT, 0, 0);
	}
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	// Load Program:
	if(!opencl.loadProgramFromFile("kernels/lorenz.cl")) {
		std::cout << "OpenCL kernel couldn't be loaded" << std::endl;
		exit();
	}
	
	// Load the kernel:
	clKernel = opencl.loadKernel("updateParticle");
	
	clMemParticles.initBuffer(sizeof(Particle) * MAX_NUM_PARTICLES, CL_MEM_READ_WRITE, particles);
	clMemNodes.initBuffer(sizeof(Node)* MAX_NUM_NODES, CL_MEM_READ_WRITE, nodes);
	clMemPosVBO.initFromGLObject(vbo[0]);
	clMemColVBO.initFromGLObject(vbo[1]);

	// Bind variables to the kernel
	clKernel->setArg(kArg_mode, currentMode);
	clKernel->setArg(kArg_particles, clMemParticles.getCLMem());
	clKernel->setArg(kArg_nodes, clMemNodes.getCLMem());
	clKernel->setArg(kArg_posBuffer, clMemPosVBO.getCLMem());
	clKernel->setArg(kArg_colBuffer, clMemColVBO.getCLMem());
	clKernel->setArg(kArg_origin, initPos);
	clKernel->setArg(kArg_color, color);
	clKernel->setArg(kArg_mousePos, mousePos);
	clKernel->setArg(kArg_dimensions, dimensions);
	clKernel->setArg(kArg_prevAvgPower, prevKickValue);
	clKernel->setArg(kArg_fftPower, kickValue);
	clKernel->setArg(kArg_numNodes, numNodes);
}
//------------------------------------------------------------------------------
void clParticles::setupOpenGL() {
#ifdef FBOS
	// Allocating FBOs:
	fboParticles.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	fboBlur.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Clearing FBOs:
	fboParticles.begin();
	{
		FBO_CLEAR;
	}
	fboParticles.end();
	
	fboBlur.begin();
	{
		FBO_CLEAR;
	}
	fboBlur.end();
#endif
	// Loading shaders:
	if(!shader.load("shaders/shader")) {
		std:cout << "Shader Can't be loaded" << std::endl;
		exit();
	}
}
//------------------------------------------------------------------------------
void clParticles::setupPosition(int i) {
    initPos.x = ofGetWidth() / 2 + radius * cos(particles[i].u);
    initPos.y = ofGetHeight() / 2 + radius * sin(particles[i].u);
	positionBuffer[i].set(initPos.x, initPos.y);
}

//------------------------------------------------------------------------------
void clParticles::setupNodes() {
	for (int i = 0; i < MAX_NUM_NODES; i++) {
		Node &node = nodes[i];
		node.pos.x = ofRandom(0, ofGetWidth());
		node.pos.y = ofRandom(0, ofGetHeight());
		node.attractForce = ofRandomf();
	}
}
//------------------------------------------------------------------------------
void clParticles::setupParticles() {
	for(int i = 0; i < MAX_NUM_PARTICLES; i++) {
		Particle &p = particles[i];
		p.vel.set(ofRandomf(), ofRandomf()); // belongs to [-1, 1]
		p.mass = ofRandom(0.1, 1);
		p.u = ofRandom(0, TWO_PI);
		setupPosition(i);
	}
}
//------------------------------------------------------------------------------
//																		  UPDATE
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::updateNodes() {

}

//------------------------------------------------------------------------------
void clParticles::updateOpenCL() {
	// Set Kernel Arguments:
	clKernel->setArg(kArg_mode, currentMode);
	clKernel->setArg(kArg_mousePos, mousePos);
	clKernel->setArg(kArg_dimensions, dimensions);
	clKernel->setArg(kArg_color, color);
	clKernel->setArg(kArg_prevAvgPower, prevKickValue);
	clKernel->setArg(kArg_fftPower, kickValue);
	clKernel->setArg(kArg_numNodes, numNodes);
	
    // Update the OpenCL kernel:
    clEnqueueAcquireGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0, 0, 0);
	clEnqueueAcquireGLObjects(opencl.getQueue(),
                              1, &clMemColVBO.getCLMem(), 0, 0, 0);
	clKernel->run1D(MAX_NUM_PARTICLES);
	clEnqueueReleaseGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0, 0, 0);
	clEnqueueReleaseGLObjects(opencl.getQueue(),
                              1, &clMemColVBO.getCLMem(), 0, 0, 0);
}

//------------------------------------------------------------------------------
//  																		DRAW
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::drawFBOs() {

}

//------------------------------------------------------------------------------
void clParticles::drawParticles() {
	glPushAttrib(GL_ENABLE_BIT);
	{
		opencl.flush(); // block previously queued OpenCL commands.

		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_BLEND_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPointSize(pointSize);
		shader.begin();
		{
#ifdef TEXTURES
			particuleTex.getTextureReference().bind();
			{
#endif
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				{
					// Bind Buffers:
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
					glVertexPointer(2, GL_FLOAT, 0, NULL);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[1]);
					glColorPointer(4, GL_FLOAT, 0, NULL);
					{
						// Draw buffers:
						glDrawArrays(GL_POINTS, 0, MAX_NUM_PARTICLES);
					}
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, NULL);
				}
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);
#ifdef TEXTURES
			}
			particuleTex.getTextureReference().unbind();
#endif
		}
		shader.end();
	}
	glPopAttrib();
}


//------------------------------------------------------------------------------
void clParticles::drawInfos() {
	glPushMatrix();
	{
		glColor3f(0.0f, 0.0f, 0.0f);
		string info = "fps: " + ofToString(ofGetFrameRate()) +
					  "\nnumber of particles: " + ofToString(MAX_NUM_PARTICLES) +
				      "\nPointSize: " + ofToString(pointSize);
		ofDrawBitmapString(info, 20, 20);
	}
	glPopMatrix();
}

//------------------------------------------------------------------------------
void clParticles::drawBeaTracker() {
//	cout << kickValue  << ","
//		 << snareValue << ","
//	     << hihatValue << endl;
}

//------------------------------------------------------------------------------
void clParticles::drawNodes() {
	ofPushMatrix();
	{
		glColor4f(color.x * kickValue,
				  color.y * snareValue,
				  color.z * hihatValue, 0.5f);
		for(int i = 0; i < MAX_NUM_NODES; i++) {
			particuleTex.draw(nodes[i].pos.x, nodes[i].pos.y, radius, radius);
		}
	}
	ofPopMatrix();
}
//------------------------------------------------------------------------------
void clParticles::drawGUI() {
	glPushMatrix();
	{
		gui->draw();
	}
	glPopMatrix();
}

//------------------------------------------------------------------------------
//																			  OF
//																	   FUNCTIONS
//------------------------------------------------------------------------------
void clParticles::setup() {
	// Initialise Parameters and global variables:
	setupParameters();
	
	// Setup Beat Tracker:
	setupBeatTracker();
	
	// Initialise the window:
	setupWindow();
		
	// Initialise the GUI components:
	setupGUI();
	
	// Initialise Nodes:
	setupNodes();
	
	// Initialise Particle System:
	setupParticles();
	
	// Initialise OpenCL:
	setupOpenCL();
	
	// Initialiser OpenGL:
	setupOpenGL();
	
	ofDisableArbTex();
	
	particuleTex.loadImage(textureName);
}
//------------------------------------------------------------------------------

void clParticles::update() {
	// Beat Tracker update:
	beatTracker.update(ofGetElapsedTimeMillis());
	kickValue = beatTracker.kick();
	snareValue = beatTracker.snare();
	hihatValue = beatTracker.hihat();
	currentMagnitude = beatTracker.getMagnitude();
	std::cout << currentMagnitude - previousMagnitude << std::endl;
	if( abs(currentMagnitude - previousMagnitude) > 0.98f && currentMode == kModeAudioReact) {
		currentMode = kModeExplode;
		currentTime = 0;
	}
	if(currentMode == kModeExplode) {
		std::cout << "kmodeExplode" << std::endl;
		if(currentTime > 0.5) {
			currentMode = kModeAudioReact;
			std::cout << "back to audio react" << std::endl;
		}
	}
	// Nodes update:
	updateNodes();
	
	// Update Arguments:
    mousePos.x   = ofGetMouseX();
	mousePos.y   = ofGetMouseY();
	dimensions.x = ofGetWidth ();
	dimensions.y = ofGetHeight();
	
	// Update OpenCL:
	updateOpenCL();
	
    // Update Global Variables:
    currentTime += dTime;
	prevKickValue = kickValue;
	previousMagnitude = currentMagnitude;
}

//------------------------------------------------------------------------------

void clParticles::draw() {
	drawParticles();
#ifdef FBOS
	drawFBOs();
#endif
	drawInfos();
	drawGUI();
	drawBeaTracker();
	drawNodes();
}

//------------------------------------------------------------------------------
//																	      EVENTS
//------------------------------------------------------------------------------
void clParticles::audioReceived(float* input, int bufferSize, int nChannels) {
	beatTracker.audioReceived(input, bufferSize, nChannels);
}

//------------------------------------------------------------------------------
void clParticles::exit() {
	gui->saveSettings("UISetting.xml");
	delete gui;
	
}

//------------------------------------------------------------------------------
void clParticles::guiEvent(ofxUIEventArgs &e) {
//	if(e.getName() == "Blur Amount") {
//		ofxUISlider *slider = e.getSlider();
//		blurAmount = slider->getScaledValue();
//	}
//	if(e.getName() == "Fade Speed") {
//		ofxUISlider *slider = e.getSlider();
//		fadeSpeed = slider->getScaledValue();
//	}
}

//------------------------------------------------------------------------------
void clParticles::keyPressed(int key) {
	if(key == OF_KEY_RIGHT) {
		pointSize++;
	}
	if(key == OF_KEY_LEFT && pointSize > 0) {
		pointSize--;
	}
	if(key == OF_KEY_UP) {
		numParticles >> 2;
		if(numParticles > MAX_NUM_PARTICLES) {
			numParticles = MAX_NUM_PARTICLES;
		}
	}
	if(key == OF_KEY_DOWN) {
		numParticles << 2;
	}
	if(key == 'v') {
		bDoVSync = !bDoVSync;
	}
	if(key == '1') {
		currentMode = kModeAudioReact;
	}
	if(key == '2') {
		currentMode = kModeExplode;
	}
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
