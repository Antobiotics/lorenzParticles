#define DAMP			0.95f
#define CENTER_FORCE	0.007f
#define MOUSE_FORCE		300.0f
#define MIN_SPEED		0.1f


typedef struct{
	float2 vel;
	float mass;
	float dummy;
    float vTheta;
    float u;
    float vU;
} Particle;

__kernel void updateParticle(__global Particle* particles, __global float2* posBuffer, const float2 mousePos, const float2 dimensions) {
	int id = get_global_id(0);
	__global Particle *p = &particles[id];
	posBuffer[id] += p->vel;
//	float2 diff = mousePos - posBuffer[id];
//	float invDistSQ = 1.0f / dot(diff, diff);
//	diff *= MOUSE_FORCE * invDistSQ;
//	
//	p->vel += (dimensions*0.5f - posBuffer[id]) * CENTER_FORCE - diff* p->mass;
//	
//	float speed2 = dot(p->vel, p->vel);
//	if(speed2<MIN_SPEED) posBuffer[id] = mousePos + diff * (1.0f + p->mass);
//	
//	posBuffer[id] += p->vel;
//	p->vel *= DAMP;
}




