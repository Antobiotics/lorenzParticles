#define DAMP			0.1//0.95f
#define CENTER_FORCE	1//0.007f
#define MOUSE_FORCE		900.0f
#define MIN_SPEED		0.3f
#define PI              3.14f

typedef struct{
	float2 vel;
	float mass;
	float dummy;
    float theta;
    float vTheta;
    float u;
    float vU;
} Particle;

__kernel void updateParticle(__global Particle* particles,  //0
                             __global float2* posBuffer,    //1
                             const float2 mousePos,         //2
                             const float2 dimensions)
{

}
//    float attractionForce = 0.7f;
//    int id = get_global_id(0);
//    __global Particle *p = &particles[id];
//    float2 pos = posBuffer[id];
//
//    float2 vecFromTargetMouse              = pos - mousePos;
//    float distToTarget = length(vecFromTargetMouse);
//    float force2 = 2;
//    p->vel += force2 * vecFromTargetMouse * attractionForce / (distToTarget + 1.0f) * p->mass;
//
//    
//    
//    float2 diff = mousePos - posBuffer[id];
//    float invDistSQ = 1.0f / dot(diff, diff);
//    diff *= MOUSE_FORCE * invDistSQ;
//
//    float speed = dot(p->vel, p->vel);
//    if( speed < MIN_SPEED)
//    {
//        posBuffer[id] = mousePos + diff * (1 + p->mass);
//    }
//
//    if( pos.x + p->vel.x > dimensions.x || pos.x + p->vel.x < 0 )
//    {
//        p->vel.x *= -1;
//    }
//    if( pos.y + p->vel.y > dimensions.y || pos.y + p->vel.y < 0)
//    {
//        p->vel.y *= -1;
//    }
//    posBuffer[id] += p->vel;
//}
//#else
//__kernel void updateParticle(__global Particle* particles,  //0
//                             __global float2* posBuffer,    //1
//                             const float2 mousePos,         //2
//                             const float2 dimensions,       //3
//                             const int    numHands,         //4
//                             const float2 hand1,            //5
//                             const float2 hand2)            //6{
//    
//    int id = get_global_id(0);
//    __global Particle *p = &particles[id];
//    float2 posBufferReduced = posBuffer[id];
//	float2 diff = mousePos - posBufferReduced;
//
//	float invDistSQ = 1.0f / dot(diff, diff);
//	diff *= MOUSE_FORCE * invDistSQ;
//	
//    float2 plop = 0.2f;
//    float2 force = CENTER_FORCE;
//    float2 summ = (dimensions * plop - posBuffer[id]) * force * force- diff* p->mass;
//    
//     p->vel += summ;
//
//    
//	float speed2 = dot(p->vel, p->vel);
//	if(speed2<MIN_SPEED)
//    {
//        posBuffer[id] = mousePos + diff * (1 + p->mass);
//    }
//    float2 buff = posBuffer[id] + diff;
//
//    posBuffer[id] += diff;//p->vel; /* - > Super bug !!*/
//	p->vel *= DAMP;
//}
//
//float absolute(float val)
//{
//    if( val < 0 )
//    {
//        return ( val * -1 );
//    }
//    return val;
//}
//#endif
