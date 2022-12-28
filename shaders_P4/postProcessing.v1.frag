#version 330 core

//Recibe coords de textura
in vec2 texCoord;

uniform sampler2D colorTex;

out vec4 colorOut;

//este hilo se va a ejecutar una vez por cada pixel de patalla

/*
//filtro gaussiano: hacemos media de todos los valores para quitar ruido
#define MASK_SIZE 9u

//desplazamiento sobre el pixel actual:
const float maskFactor = float (1.0/14.0);
const vec2 texIdx[MASK_SIZE] = vec2[](
vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0),
vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0),
vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0));
//esto es como lo q hacia en fundamentos: -1, 0 es el q esta a la izq de mi punto, etc..

const float mask[MASK_SIZE] = float[](
float (1.0*maskFactor), float (2.0*maskFactor), float (1.0*maskFactor),
float (2.0*maskFactor), float (2.0*maskFactor), float (2.0*maskFactor),
float (1.0*maskFactor), float (2.0*maskFactor), float (1.0*maskFactor));
//factor de la mascara sirve para mantener q sumen como max 1 para q no gane energia
//hace q todos los valores sumen 1

//esta mascara coge valor actual y lo multiplica por 4 y vecinos por -1
const float mask[MASK_SIZE] = float[](
0.0, -1.0, 0.0,
-1.0, 4.0, -1.0,
0.0, -1.0, 0.0);
//voy a tener un valor negro ya q el pixel va a ser igual a sus vecinos
//si tiene mayor valor el pixel se va a ver
//solo se van a ver los bordes --> pixeles de intensidd alta rodeados de pixeles de intensidad baja
*/


//Mascara q tenga en cuenta 25 vecinos --> a cada peso le corresponde un vecino
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

//esto lo vamos a usar para implementar filtro de profundidad de campo

void main()
{
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0)); //textureSixe: numero de pixeles de alto y de ancho
	//calculo inversa del vector
	//indexo textura con valor entre 0 y 1
	//incremento en coords de textura para cada vecino
	
	vec4 color = vec4 (0.0);
	
	for (uint i = 0u; i < MASK_SIZE; i++) //recorro los dos vectores q tengo arriba
	{
		vec2 iidx = texCoord + ts * texIdx[i]; //cojo texel y le sumo coords del actual --> 1 partido del num de texeles creo
		color += texture(colorTex, iidx,0.0) * mask[i]; //acceder a la textura
	}
	
	colorOut = color;
}

//con esto imagen se ve borrosa




