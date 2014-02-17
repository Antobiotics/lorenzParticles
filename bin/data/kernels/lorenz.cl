#define DAMP			0.2f
#define CENTER_FORCE	0.037f
#define MOUSE_FORCE		300.0f
#define MIN_SPEED		0.1f

#define kArg_particles  0
#define kArg_posBuffer  1
#define kArg_mousePos   2
#define kArg_dimensions 3
#define kArg_parameters 4
#define kArg_timeStep   5
#define kArg_dTime		6

typedef struct{
	float2 vel;
	float mass;
	float dummy;
    float theta;
    float vTheta;
    float u;
    float vU;
} Particle;

__kernel void updateParticle(__global Particle* particles ,   //0
							 __global float2*   posBuffer ,   //1
								const float2    mousePos  ,   //2
								const float2    dimensions,   //3
								const float4    parameters,   //4
							    const float     timeStep  ,   //5
							    const float     dTime     )   //6
{
//	int id = get_global_id(0);
//	__global Particle *p = &particles[id];
//
//	float2 diff = mousePos - posBuffer[id];
//	float invDistSQ = 1.0f / dot(diff, diff);
//	diff *= MOUSE_FORCE * invDistSQ;
//
//	p->vel += (dimensions * 0.5f - posBuffer[id]) * CENTER_FORCE * p->mass * p->mass;
//
//	float speed2 = dot(p->vel, p->vel);
//	if(speed2<MIN_SPEED) posBuffer[id] = mousePos + diff * (1.0f + p->mass);
//
//	posBuffer[id] += p->vel;
//	p->vel *= DAMP;
//	posBuffer[id] = posBuffer[id] + p->vel;
}




