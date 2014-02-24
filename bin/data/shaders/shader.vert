#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

void main() {
	
    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;
	float size    = gl_Normal.x;
    gl_PointSize  = size;
    gl_FrontColor = gl_Color;

}

//attribute vec3 aVertexPosition;
//attribute vec4 aVertexColor;

//uniform mat4 uMVMatrix;
//uniform mat4 uPMatrix;
//varying vec4 vColor;
//void main(void) {
//	gl_Position = uPMatrix * uMVMatrix;
//	vColor = aVertexColor;
//}
