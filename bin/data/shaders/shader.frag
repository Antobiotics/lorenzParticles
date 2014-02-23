#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D tex;

void main (void) {
	gl_FragColor = texture2D(tex, gl_TexCoord[0].st) * gl_Color;
}