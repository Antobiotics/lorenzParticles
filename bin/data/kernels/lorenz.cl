//------------------------------------------------------------------------------
#define DAMP			0.92f
#define CENTER_FORCE	0.037f
#define MOUSE_FORCE		300.0f
#define MIN_SPEED		0.1f
#define PI              3.1415926

//------------------------------------------------------------------------------
typedef struct{
	float2 vel;
	float mass;
	float u;
} Particle;

//------------------------------------------------------------------------------
typedef struct {
	float2 pos;
	float frequency;
	float attractForce;
} Node;

//------------------------------------------------------------------------------
float rand(float2 co) {
	float i;
	return fabs(fract(sin(dot(co.xy ,make_float2(12.9898f, 78.233f))) * 43758.5453f, &i));
}

//------------------------------------------------------------------------------
__kernel void updateParticle(__global Particle* particles     ,
							 __global Node*     nodes         ,
							 __global float2*   posBuffer     ,
							 __global float4*   colBuffer     ,
							    const unsigned int currentMode,
							 	const float2    origin        ,
								const float4    color         ,
								const float2    mousePos      ,
								const float2    dimensions    ,
							 	const float     prevAvgPower  ,
							 	const float     avgPower      ,
							 	const unsigned int numNodes   )
{
	int id = get_global_id(0);
	__global Particle *p = &particles[id];
	float2 currentPos = posBuffer[id];
	if(currentMode == 0) {
		// Audio Reactive Mode:
		float2 newPos;
		float pAngle = p->u;

		float AmpFactor = 40;
		float fftPower = 0.1;
		fftPower = prevAvgPower - avgPower * 0.9;

		newPos.x = origin.x + (fftPower * AmpFactor * p->mass + 35) * cos(pAngle);
		newPos.y = origin.y + (fftPower * AmpFactor * p->mass + 35) * sin(pAngle);
		
		p->vel += (newPos - currentPos) * fftPower * p->mass * p->mass;
		
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
		colorR.y = fftPower * 1.5;
		colBuffer[id] = colorR * 0.95;
	} else {
		// Node attractors mode

		int	birthNodeId	= id % numNodes;
		float2 vecFromBirthNode	= currentPos - nodes[birthNodeId].pos;
		float distToBirthNode = fast_length(vecFromBirthNode);
		
		int targetNodeId = (id % 2 == 0) ? (id + 1) % numNodes : (id + numNodes - 1) % numNodes;
		float2 vecFromTargetNode = currentPos - nodes[targetNodeId].pos;
		float distToTargetNode = fast_length(vecFromTargetNode);

		float2 diffBetweenNodes	= nodes[targetNodeId].pos - nodes[birthNodeId].pos;
		float2 normBetweenNodes	= fast_normalize(diffBetweenNodes);
		float distBetweenNodes = fast_length(diffBetweenNodes);

		float	dotTargetNode		= fmax(0.0f, dot(vecFromTargetNode, -normBetweenNodes));
		float	dotBirthNode		= fmax(0.0f, dot(vecFromBirthNode, normBetweenNodes));
		float	distRatio			= fmin(1.0f, fmin(dotTargetNode, dotBirthNode) / (distBetweenNodes * 0.5f));

		// add attraction to other nodes
		p->vel -= vecFromTargetNode * nodes[targetNodeId].attractForce / (distToTargetNode + 1.0f) * p->mass;
		
//		// add wave
//		float2 waveVel = make_float2(-normBetweenNodes.y, normBetweenNodes.x) * sin(time + 10.0f * 3.1416926f * distRatio * nodes[birthNodeId].waveFreq);
//		float2 sideways				= nodes[birthNodeId].waveAmp * waveVel * distRatio * p->mass;
//		posBuffer[id]				+= sideways * wavePosMult;
//		p->vel						+= sideways * waveVelMult * dotTargetNode / (distBetweenNodes + 1);
//		
		// set color
//		float invLife = 1.0f - p->life;
//		colBuffer[id] = color * (1.0f - invLife * invLife * invLife);// * sqrt(p->life);	// fade with life
		
		// add waviness
		if(distToTargetNode < 1.0f) {
			posBuffer[id] = nodes[birthNodeId].pos;
			float a = rand(p->vel) * 3.1415926f * 30.0f;
			float r = rand(currentPos);
//			p->vel = make_float2(cos(a), sin(a)) * (nodes[birthNodeId].spread * r * r * r);
			p->vel = make_float2(cos(a), sin(a)) * (r * r * r);
//			p->life = 1.0f;
//			//		p->mass = mix(massMin, 1.0f, r);
		} else {
			posBuffer[id] = currentPos;
			colBuffer[id] = colBuffer[id] * 0.96;
			
			posBuffer[id] += p->vel;
//			p->vel *= 0.22;
		}
	}
}

//------------------------------------------------------------------------------



