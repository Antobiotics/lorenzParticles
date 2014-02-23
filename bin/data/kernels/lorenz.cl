//------------------------------------------------------------------------------
#define DAMP			0.95f
#define CENTER_FORCE	0.037f
#define MOUSE_FORCE		300.0f
#define MIN_SPEED		0.1f
#define PI              3.1415926

//------------------------------------------------------------------------------
#define kArg_particles  0
#define kArg_posBuffer  1
#define kArg_mousePos   2
#define kArg_dimensions 3
#define kArg_parameters 4
#define kArg_timeStep   5
#define kArg_dTime		6

//------------------------------------------------------------------------------
typedef struct{
	float2 vel;
	float mass;
	float u;
} Particle;

//------------------------------------------------------------------------------
typedef struct {
	float2 pos;
	float spread;
	float attractForce;
} Node;

//------------------------------------------------------------------------------
float rand(float2 co) {
	float i;
	return fabs(fract(sin(dot(co.xy ,make_float2(12.9898f, 78.233f))) * 43758.5453f, &i));
}

//------------------------------------------------------------------------------
__kernel void updateParticle(__global Particle* particles   ,
							 __global Node*     nodes       ,
							 __global float2*   posBuffer   ,
							 __global float4*   colBuffer   ,
							 	const float2    origin      ,
								const float4    color       ,
								const float2    mousePos    ,
								const float2    dimensions  ,
							 	const float     prevAvgPower,
							 	const float     avgPower    )
{
	int id = get_global_id(0);
	__global Particle *p = &particles[id];
	float2 currentPos = posBuffer[id];
	float2 newPos;
	float pAngle = p->u;

	float AmpFactor = 60;
	float fftPower = 0.1;
	fftPower = prevAvgPower - avgPower * 0.1;
//	if( avgPower > 0) {
//		fftPower = avgPower;
//	}
	newPos.x = origin.x + (fftPower * AmpFactor * p->mass + 35) * cos(pAngle);
	newPos.y = origin.y + (fftPower * AmpFactor * p->mass + 35) * sin(pAngle);
	
	p->vel += (newPos - currentPos) * fftPower * p->mass;
	
	if(newPos.x > dimensions.x || newPos.x < 0) {
		newPos.x = currentPos.x - p->vel.x;
		p->vel.x *= -1.0f;
	}

	if(newPos.y > dimensions.y || newPos.y < 0) {
		newPos.y = currentPos.y - p->vel.y;
		p->vel.y *= -1.0f;
	}
	
	posBuffer[id] += p->vel;
	p->vel *= DAMP;
	
	// Colors:
	float4 colorR = color;
//	colorR.y = fftPower * 1.2;
	colBuffer[id] = colorR;
}

//------------------------------------------------------------------------------



