//----------------------
#include "clParticles.h"
//----------------------

#define MAX_NUM_PARTICLES ( 512 * 512 )

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

Particle particles[MAX_NUM_PARTICLES];
msa::OpenCLBuffer clMemParticles; // Stores particles.
float2 particlesPos[MAX_NUM_PARTICLES];
msa::OpenCLBuffer clMemPosVBO; // Stores particlesPos.
msa::OpenCLKernel *clKernel;

GLuint vbo[1];

ofColor backgroundColor;
float4 parameters;
float2 mousePos;
float2 dimensions;
float currentTime;
float dTime;

unsigned int numParticles;
unsigned int pointSize;
float fadeSpeed;
float blurAmount;
float radius;

string textureName = "glitter.png";

ofImage particuleTex;

ofFbo fboBlur;
ofFbo fboPrev;
ofFbo fboParticles;
ofShader shader;

ofxUICanvas *gui;

//------------------------------------------------------------------------------
//																		   SETUP
//																	   FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void clParticles::setupWindow() {
	ofBackground(backgroundColor);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(false);
}

//------------------------------------------------------------------------------
void clParticles::setupGUI() {
	gui = new ofxUICanvas();
	gui->init(0, ofGetHeight()/8, ofGetWidth()/6, ofGetHeight());
	gui->addSlider("Blur Amount", 0, 50, blurAmount);
	gui->addSlider("Fade Speed", 0, 1, fadeSpeed);
	
	ofAddListener(gui->newGUIEvent, this, &clParticles::guiEvent);
	gui->autoSizeToFitWidgets();
	gui->loadSettings("UISettings.xml");
}
//------------------------------------------------------------------------------
void clParticles::setupParameters() {
	// Colors:
	backgroundColor = *new ofColor(255, 255, 255);
	
	numParticles = MAX_NUM_PARTICLES;
	
	// Shader Parameters:
	fadeSpeed = 0.25f;
	blurAmount = 1;
	
	// Time variables:
	currentTime = 0;
	dTime = 0.01;
	
	// Dimensions:
	pointSize = 1;
	radius = 200;
}
//------------------------------------------------------------------------------
void clParticles::setupOpenCL() {
	opencl.setupFromOpenGL();
	
	// Bind buffer to the particle position.
	glGenBuffersARB(1, vbo); // Create a new buffer object.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]); // Bind the buffer object.
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,
					sizeof(float2) * MAX_NUM_PARTICLES,
					particlesPos,
					GL_DYNAMIC_COPY_ARB); // Copy data to the buffer object.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	// Load Program:
	opencl.loadProgramFromFile("kernels/lorenz.cl");
	
	// Load the kernel:
	clKernel = opencl.loadKernel("updateParticle");
	
	clMemParticles.initBuffer(sizeof(Particle) * MAX_NUM_PARTICLES, CL_MEM_READ_WRITE, particles);
	clMemPosVBO.initFromGLObject(vbo[0]);

	// Bind variables to the kernel
	clKernel->setArg(kArg_particles, clMemParticles.getCLMem());
	clKernel->setArg(kArg_posBuffer, clMemPosVBO.getCLMem());
	clKernel->setArg(kArg_mousePos, mousePos);
	clKernel->setArg(kArg_dimensions, dimensions);
}
//------------------------------------------------------------------------------
void clParticles::setupOpenGL() {
	fboParticles.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	fboBlur.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
	
	if(shader.load("shaders/blur2")) {
		std:cout << "Shader loaded" << std::endl;
	}
	glPointSize(2);
}
//------------------------------------------------------------------------------
void clParticles::setupPosition(int i) {
    float initPosX = ofRandom(0, ofGetWidth());
    float initPosY = ofRandom(0, ofGetHeight());
	particlesPos[i].set(initPosX, initPosY);
}

//------------------------------------------------------------------------------
void clParticles::setupParticles() {
	for(int i = 0; i < MAX_NUM_PARTICLES; i++) {
		Particle &p = particles[i];
		p.vel.set(0, 0);
		p.mass = ofRandom(0.5, 1);
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
void clParticles::drawFBOs() {
//	drawParticles();
	fboParticles.begin();
	{
		ofClear(255, 255, 255);
		drawParticles();
	}
	fboParticles.end();
//	fboParticles.draw(0, 0);
//	fboPrev.begin();
//	{
//		glEnable(GL_BLEND);
//		glBlendFunc(GL_ONE, GL_ONE);
//		float color = 1.0f - fadeSpeed;
//		
//		glColor4f(color, color, color, 1.0f);
//		fboBlur.draw(0, 0, fboPrev.getWidth(), fboPrev.getHeight());
//		
//		glColor3f(1.0f, 1.0f, 1.0f);
//		fboNew.draw(0, 0, fboPrev.getWidth(), fboPrev.getHeight());
//	}
//	fboPrev.end();
	
	fboBlur.begin();
	{
		ofClear(255, 255, 255);
//		shaderBlurX.begin();
//		{
//			ofClear(0, 0, 0);
//			shaderBlurX.setUniform1f("blurAmnt", blurAmount);
//		}
//		shaderBlurX.end();
		fboParticles.draw(0, 0);
	}
	fboBlur.end();
	fboBlur.draw(0, 0);
}

//------------------------------------------------------------------------------
void clParticles::drawParticles() {
	glPushAttrib(GL_ENABLE_BIT);
	{
		opencl.finish(); // block previously queued OpenCL commands.

			glColor3f(0.0f, 0.0f, 0.0f);
			glEnable(GL_POINT_SPRITE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
			glEnable(GL_POINT_SMOOTH);
			glEnable(GL_BLEND);
			glBlendFunc(GL_BLEND_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glPointSize(pointSize);

			glEnableClientState(GL_VERTEX_ARRAY);
		shader.begin();
		{
			shader.setUniform1f("amountX", 1 / ofGetWidth() * blurAmount);
			shader.setUniform1f("amountY", 1 / ofGetHeight() * blurAmount);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
			{
				glVertexPointer(2, GL_FLOAT, 0, NULL);
//				particuleTex.getTextureReference().bind();
				{
					glDrawArrays(GL_POINTS, 0, MAX_NUM_PARTICLES);
				}
//				particuleTex.getTextureReference().unbind();
			}
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, NULL);
		}
		shader.end();
	}
	glPopAttrib();
}

//------------------------------------------------------------------------------
void clParticles::drawInfos() {
	glColor3f(0.0f, 0.0f, 0.0f);
	string info = "fps: " + ofToString(ofGetFrameRate()) +
				  "\nnumber of particles: " + ofToString(MAX_NUM_PARTICLES) +
				  "\nPointSize: " + ofToString(pointSize);
	ofDrawBitmapString(info, 20, 20);
}

//------------------------------------------------------------------------------
void clParticles::drawGUI() {
	gui->draw();
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

	// Initialise the GUI components:
	setupGUI();
	
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
	// Update Arguments:
    mousePos.x   = ofGetMouseX();
	mousePos.y   = ofGetMouseY();
	dimensions.x = ofGetWidth ();
	dimensions.y = ofGetHeight();
	
	// Set Kernel Arguments:
	clKernel->setArg(kArg_mousePos, mousePos);
	clKernel->setArg(kArg_dimensions, dimensions);

    // Update the OpenCL kernel:
    clEnqueueAcquireGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0, 0, 0);
	clKernel->run1D(MAX_NUM_PARTICLES);
	clEnqueueReleaseGLObjects(opencl.getQueue(),
                              1, &clMemPosVBO.getCLMem(), 0, 0, 0);
    
    // Update Global Variables:
    currentTime += dTime;
}

//------------------------------------------------------------------------------

void clParticles::draw() {
	drawFBOs();
	drawInfos();
	drawGUI();
}

//------------------------------------------------------------------------------
//																	      EVENTS
//------------------------------------------------------------------------------
void clParticles::exit() {
	gui->saveSettings("UISetting.xml");
	delete gui;
}

//------------------------------------------------------------------------------
void clParticles::guiEvent(ofxUIEventArgs &e) {
	if(e.getName() == "Blur Amount") {
		ofxUISlider *slider = e.getSlider();
		blurAmount = slider->getScaledValue();
	}
	if(e.getName() == "Fade Speed") {
		ofxUISlider *slider = e.getSlider();
		fadeSpeed = slider->getScaledValue();
	}
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
