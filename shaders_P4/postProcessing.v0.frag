#version 330 core

//Recibe coords de textura
in vec2 texCoord;

uniform sampler2D colorTex;
out vec4 colorOut;

void main()
{
	//colorOut = vec4(texCoord, vec2(0.0)); //renderizamos coords de textura pa comprobar si estan bn creadas

	colorOut = vec4(textureLod(colorTex, texCoord, 0.0).rgb, 0.6);  //acceder a textura --> funcion (identificador variable textura, coords de la textura, nivel del midmap)
	//accedemos a nivel 0 de midmap porq es la unica imagen con info, el resto estan vacías
	//para dar mas peso al actual le damos mas valor al midmap
}