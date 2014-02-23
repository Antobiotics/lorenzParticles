#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D tex;

void main (void) {
	gl_FragColor = vec4( 0, 1, 0, 1 );
}