#version 330 core


in vec2 texCoord;
uniform sampler2D colorTex;
uniform sampler2D zCamTex;
out vec4 colorOut;

const float focalDistance = -25.0;
const float maxDistanceFactor = 1.0 / 5.0;

//#define MASK_SIZE 9u
//const vec2 texIdx[MASK_SIZE] = vec2[](
//vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0),
//vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0),
//vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0));
//const float maskFactor = float (1.0/14.0);
//const float mask[MASK_SIZE] = float[](
//float (1.0*maskFactor), float (2.0*maskFactor), float (1.0*maskFactor),
//float (2.0*maskFactor), float (2.0*maskFactor), float (2.0*maskFactor),
//float (1.0*maskFactor), float (2.0*maskFactor), float (1.0*maskFactor));
//const float mask[MASK_SIZE] = float[](
//0.0,-1.0,0.0,
//-1.0, 4.0,-1.0,
//0.0, -1.0, 0.0);

#define MASK_SIZE 25u
const vec2 texIdx[MASK_SIZE] = vec2[](
vec2(-2.0,2.0), vec2(-1.0,2.0), vec2(0.0,2.0), vec2(1.0,2.0), vec2(2.0,2.0),
vec2(-2.0,1.0), vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0), vec2(2.0,1.0),
vec2(-2.0,0.0), vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0), vec2(2.0,0.0),
vec2(-2.0,-1.0), vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0), vec2(2.0,-1.0),
vec2(-2.0,-2.0), vec2(-1.0,-2.0), vec2(0.0,-2.0), vec2(1.0,-2.0), vec2(2.0,-2.0));
const float maskFactor = float (1.0/65.0);
const float mask[MASK_SIZE] = float[](
1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor,2.0*maskFactor, 1.0*maskFactor,
2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor,3.0*maskFactor, 2.0*maskFactor,
3.0*maskFactor, 4.0*maskFactor, 5.0*maskFactor,4.0*maskFactor, 3.0*maskFactor,
2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor,3.0*maskFactor, 2.0*maskFactor, 
1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor,2.0*maskFactor, 1.0*maskFactor);

void main()
{
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2(textureSize(colorTex, 0));
	float dof = abs(texture(zCamTex, texCoord).x - focalDistance)
		* maxDistanceFactor;
	dof = clamp(dof, 0.0, 1.0);
	dof *= dof;
	vec4 color = vec4(0.0);
	for (uint i = 0u; i < MASK_SIZE; i++)
	{
		vec2 iidx = texCoord + ts * texIdx[i] * dof;
		color += texture(colorTex, iidx, 0.0) * mask[i];
	}
	colorOut = color;
}