#ifndef lorenzParticles_particle_h
#define lorenzParticles_particle_h

//------------------------------------------------------------------------------
typedef struct{
	float2 vel;
	float mass;
	float dummy;
    float theta;
    float vTheta;
    float u;
    float vU;
} Particle;

//------------------------------------------------------------------------------
#endif
