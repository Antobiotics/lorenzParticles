#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D tex;
uniform float timeStep;
uniform vec2  mouse;


void main (void) {
    vec2 st = gl_TexCoord[0].st;
    vec4 pos = texture2D(tex, st);

    pos += 0.01;

    gl_FragColor = gl_Color;
}