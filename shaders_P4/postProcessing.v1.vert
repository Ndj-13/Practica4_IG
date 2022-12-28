#version 330 core

//objetivo: dar valor a glposition y generar coords de textura

layout(location = 0)in vec3 inPos;	//antes de lincar shader --> asignar a atributos identificadores --> layout location

//Variables Variantes
out vec2 texCoord;

void main()
{
	//Código del Shader
	texCoord = inPos.xy*0.5+vec2(0.5); //paso de rango de -1,1 a 0,1
	gl_Position = vec4 (inPos,1.0);
}
