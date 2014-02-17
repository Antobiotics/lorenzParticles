#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

uniform float timeStep;
uniform vec2  mouse;

void main() {

    vec4 currentVert = gl_Vertex;
    vec4 outputVert  = currentVert;

    float redx = (outputVert.x ) / 1024;
    float redy = (outputVert.y ) / 1024;
    gl_Position = gl_ModelViewProjectionMatrix * outputVert;
    float size    = gl_Normal.y;
    gl_PointSize  = size;
    gl_FrontColor = gl_Color * vec4(redy,redx,0.5,1.0);

}