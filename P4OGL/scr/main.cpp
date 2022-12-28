#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>

#define RAND_SEED 31415926
#define SCREEN_SIZE 500,500
#define MASK_SIZE 25

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f); //pasar de coords del mundo a camara
glm::mat4 model = glm::mat4(1.0f); //de coords del modelo a camara
//memoria del cliente y del servidor separadas --> hay q suirlas a tarjeta grafica


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
float angle = 0.0f;

//VAO
unsigned int vao; //como interpretar vbos

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int colorTexId;
unsigned int emiTexId;

unsigned int planeVAO;
unsigned int planeVertexVBO;

//Por definir
unsigned int vshader; 
unsigned int fshader;
unsigned int program; //shaders q trabajan de forma conjunta se tienen q enlazar en un programa

//Variables Uniform --> identificadores de las matrices uniform del shader
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

//Texturas Uniform --> a diferencia de las otras (colorTexId y emiTexId), estas no son texturas sino variables del shader
//unas son la textura y otras las variables q usa el shader para accecder a la textura
int uColorTex;
int uEmiTex;

//Atributos 
int inPos;
int inColor;
int inNormal;
int inTexCoord;

unsigned int postProccesVShader;
unsigned int postProccesFShader;
unsigned int postProccesProgram;
//Uniform
unsigned int uColorTexPP; //acceso a variable uniforme
unsigned int uDepthTexPP;
unsigned int uVertexTexPP;
//Atributos
int inPosPP; //acceso a atributo

//creamos identificadores para fbo y las 2 texturas (color y profundidad)
unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int vertexBuffTexId;
unsigned int depthBuffTexId;

//Segundo fbo para varios filtros gausianos
unsigned int fbo2;

//Variable para identificar textura deprofundidad:
unsigned int zCamBuffTexId;

//Variables
float mb = 0.6; //motion blur
float mbcolor = 0.5; 


int udfocal; //identificador distancia del foco
float dfocal = -25.0;
int udmax;
float dmax = 1.0 / 5.0;

int udnear;
float dnear = 1;
int udfar;
float dfar = 30;

int umaskF;
int umask;
float maskF = float(1.0 / 65.0);
float* mask = new float[MASK_SIZE] {
		1.0f * maskF, 2.0f * maskF, 3.0f * maskF, 2.0f * maskF, 1.0f * maskF,
		2.0f * maskF, 3.0f * maskF, 4.0f * maskF, 3.0f * maskF, 2.0f * maskF,
		3.0f * maskF, 4.0f * maskF, 5.0f * maskF, 4.0f * maskF, 3.0f * maskF,
		2.0f * maskF, 3.0f * maskF, 4.0f * maskF, 3.0f * maskF, 2.0f * maskF,
		1.0f * maskF, 2.0f * maskF, 3.0f * maskF, 2.0f * maskF, 1.0f * maskF };

//Detectar bordes


//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

void renderCube();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char *vname, const char *fname);
void initShaderPP(const char* vname, const char* fname);
void initObj();
void initPlane();
void resizeFBO(unsigned int w, unsigned int h);
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// Nuevas variables auxiliares
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Nuevas funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShaderFw("../shaders_P4/fwRendering.v0.vert", "../shaders_P4/fwRendering.v0.frag");
	initObj();
	initShaderPP("../shaders_P4/postProcessing.vMask.vert", "../shaders_P4/postProcessing.vOp1.frag");
	initPlane();
	
	//Las 2 texturas q necesitamos (color y profundidad) --> les damos identificador
	glGenFramebuffers(1, &fbo); 
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);
	glGenTextures(1, &vertexBuffTexId);
	glGenTextures(1, &zCamBuffTexId);

	glGenFramebuffers(1, &fbo2);

	resizeFBO(SCREEN_SIZE); //reservar espacio
	//lo hacemos con esta funcion para q cada vez q cambie tamaño de la pantalla, cambie el de las texturas --> se adapte

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -25.0f;
}


void destroy()
{
	
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);
	
	 /*
	glDetachShader(postProccesProgram, postProccesVShader);
	glDetachShader(postProccesProgram, postProccesFShader);
	glDeleteShader(postProccesVShader);
	glDeleteShader(postProccesFShader);
	glDeleteProgram(postProccesProgram);
	*/

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
	glDeleteTextures(1, &colorBuffTexId);
	glDeleteTextures(1, &depthBuffTexId);
	glDeleteTextures(1, &vertexBuffTexId);

}

void initShaderFw(const char *vname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");


	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}


	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 2,
			cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex*sizeof(unsigned int) * 3, cubeTriangleIndex,
		GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compilo bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char *fileName)
{
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc() //pinta en el default frame buffer porq es como esta configurado --> maquina de estados
{
	//Activar fbo 
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//primero activamos y luego limpiamos pa q no limpie el default frame buffer

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //q limpie color y profundidad

	
	glUseProgram(program);

	//Texturas
	if (uColorTex != -1) //si variable colorTex se utiliza
	{
		glUniform1i(uColorTex, 0); //coge este color creo
	}
	glActiveTexture(GL_TEXTURE0); //activa el puerto 0 (unidad de texturas 0)
	glBindTexture(GL_TEXTURE_2D, colorTexId); //activo color

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}


	model = glm::mat4(2.0f);
	model[3].w = 1.0f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));

	glm::mat4 m2 = model;

	//renderCube(); //coge matriz model y view, la sube al vertice, activa vao y no se q

	std::srand(RAND_SEED); //generador de numeros aleatorios
	for (unsigned int i = 0; i < 10; i++)
	{
		float size = float(std::rand() % 3 + 1);

		glm::vec3 axis(glm::vec3(float(std::rand() % 2),
			float(std::rand() % 2), float(std::rand() % 2)));
		if (glm::all(glm::equal(axis, glm::vec3(0.0f))))
			axis = glm::vec3(1.0f);

		float trans = float(std::rand() % 7 + 3) * 1.00f + 0.5f;
		glm::vec3 transVec = axis * trans;
		transVec.x *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.y *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.z *= (std::rand() % 2) ? 1.0f : -1.0f;

		model = glm::rotate(glm::mat4(1.0f), angle*2.0f*size, axis);
		model = glm::translate(model, transVec);
		model = glm::rotate(model, angle*2.0f*size, axis);
		model = glm::scale(model, glm::vec3(1.0f / (size*0.7f)));
		renderCube();
	}

	model = m2; 
	renderCube();

	//en esta segunda pasada lo primero q hacemos es activar default framen buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//comprobamos si plano funciona --> pintamos cualquier cosa:
	//activar shader y vao:
	glUseProgram(postProccesProgram);

	if (udmax != -1) glUniform1fv(udmax, 1, &dmax);
	if (udfocal != -1) glUniform1fv(udfocal, 1, &dfocal);

	if (udnear != -1) glUniform1fv(udnear, 1, &dnear);
	if (udfar != -1) glUniform1fv(udfar, 1, &dfar);

	if (umaskF != -1) glUniform1fv(umaskF, 1, &maskF);
	if (umask != -1) glUniform1fv(umask, 25, mask);


	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Esto ya lo hemos hecho en primera pasada asiq lo limpieamos:
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//vamos a sobreescribirlo asiq no hace falta limpiar

	
	//Blending para el motion bur
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //factor por el q multiplicamos la fuente (alpha) y factor por el q multiplicamos el destino (1-alpha)
	//glBlendEquation(GL_FUNC_ADD); //suma los fragmentos q llegan
	
	/*
	float SRC; //color del fragmento q llega
	float fsrc; //valor q multiplica al color del fragmento q llega para calcular el color
	float fdist; //distancia de la camara
	float DST; //pixel
	calcular color: c = fsrc*SRC<op>fdist*DST
	*/

	glBlendFunc(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA);
	glBlendColor(mbcolor, mbcolor, mbcolor, mb);
	glBlendEquation(GL_FUNC_ADD);
	
	//activamos textura emisiva
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, emiTexId); //al iniciar programa le dijimos donde buscar textura en initFunc por eso no hace falta volver a decirselo

	//queremos activar textura de color q metimos en fbo asiq cambiamos emi por color
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId); //al iniciar programa le dijimos donde buscar textura en initFunc por eso no hace falta volver a decirselo

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
	

	//lo mismo para el desenfoque de profundidad
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, zCamBuffTexId);

	glBindVertexArray(planeVAO);
	//glDrawElements(); --> no podemos usar esta porq estamos usando index
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //interpretar lista de vertices como strips de triangulos, empezamos por vertice 0, cuantos vertices hay (4)

	glDisable(GL_BLEND);
	//lo volvemos a iniciar para la siguiente pasada
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	glutSwapBuffers();
}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
		&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
		&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
		&(normal[0][0]));
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);
}



void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) /float(height), 1.0f, 50.0f);

	resizeFBO(width, height);

	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.00085f;
	
	glutPostRedisplay();
}

void initShaderPP(const char* vname, const char* fname) 
{
	postProccesVShader = loadShader(vname, GL_VERTEX_SHADER);
	postProccesFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	postProccesProgram = glCreateProgram();
	glAttachShader(postProccesProgram, postProccesVShader);
	glAttachShader(postProccesProgram, postProccesFShader);
	//glBindAttribLocation(postProccesProgram, 0, "inPos"); --> no hace falta, lo hicimos dentro del shader

	glLinkProgram(postProccesProgram); //comprobar linkado: si ha habido error --> gestion problemas
	int linked;
	glGetProgramiv(postProccesProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProccesProgram, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(postProccesProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(postProccesProgram);
		postProccesProgram = 0;
		exit(-1);
	}
	//Preguntar por identificadores:
	uColorTexPP = glGetUniformLocation(postProccesProgram, "colorTex");
	inPosPP = glGetAttribLocation(postProccesProgram, "inPos");

	udfocal = glGetUniformLocation(postProccesProgram, "dfocal");
	udmax = glGetUniformLocation(postProccesProgram, "dmax");

	udnear = glGetUniformLocation(postProccesProgram, "dnear");
	udfar = glGetUniformLocation(postProccesProgram, "dfar");

	uVertexTexPP = glGetUniformLocation(postProccesProgram, "vertexTex");
	uDepthTexPP = glGetUniformLocation(postProccesProgram, "depthTex");

	//creamos variable local:
	int uZCamTex = glGetUniformLocation(postProccesProgram, "zCamTex");

	umaskF = glGetUniformLocation(postProccesProgram, "maskF");
	umask = glGetUniformLocation(postProccesProgram, "mask");


	glUseProgram(postProccesProgram); //lo activamos para configurar variable uniforme

	if (uColorTexPP != -1) glUniform1i(uColorTexPP, 0); //Enlazar a un puerto --> siempre q haya q acceder a colorTecPP mira en el puerto 0
	//lo decimos una vez y ya vale pa siempre

	if (uVertexTexPP != -1) glUniform1i(uVertexTexPP, 2);

	if (uDepthTexPP != -1) glUniform1i(uDepthTexPP, 1);

	if (uZCamTex != -1) glUniform1i(uZCamTex, 3);
	 
}

void initPlane()
{
	glGenVertexArrays(1, &planeVAO); //cramos vao
	glBindVertexArray(planeVAO); //activamos vao pa configurarlo

	glGenBuffers(1, &planeVertexVBO); //creamos vbo
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO); //activamos vbo
	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
		planeVertexPos, GL_STATIC_DRAW); //reservamos espacio para vbo y subimos fichero de plane 

	glVertexAttribPointer(inPosPP, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(inPosPP);
}

void resizeFBO(unsigned int w, unsigned int h)
{
	//Estas 2 funciones: activar textura (para modificarla) y reservar espacio a la textura
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		GL_RGBA, GL_FLOAT, NULL); //estos definen formato externo --> subir datos --> estan en formato RGBA cada uno en tipo float
									//formato externo e interno deben ser compatibles
	
	/*
	//Estas definien como acceder a las texturas:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //para el desenfoque hay q cambiar esto pa q sea lineal, no coja el mas cercano
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	*/

	//Estas definien como acceder a las texturas:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //para el desenfoque hay q cambiar esto pa q sea lineal, no coja el mas cercano
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	//Activo textura de profundidad (usamos formato de profundidad en vez de de color)
	//fbo tiene varias salidas de color, una de profundidad y una de stencil creo
	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL); //textura de profundidad tiene q tener formato de profundidad de 24 bits, no vale color
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//para profundidad:
	glBindTexture(GL_TEXTURE_2D, zCamBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, /*GL_RGB32F*/ GL_R32F, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, 0); // no hacen faltta tantos canales
	//configurar como acceder a texturas:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	 
	//lo metemos en un atachment de color

	glBindFramebuffer(GL_FRAMEBUFFER, fbo); //activamos frame buffer obj para configurarlo

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, colorBuffTexId, 0); //color attachment, 0 = nivel del midmap

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, zCamBuffTexId, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		depthBuffTexId, 0); //lo pone en el atachment de profundidad una vez le hemos puesto formato de profundidad

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
		GL_TEXTURE_2D, vertexBuffTexId, 0);

	//Asignar attachments a salidas
	//const GLenum buffs[2] = { GL_NONE, GL_COLOR_ATTACHMENT0 };  //hacemos un buffer de 2 componentes (Aqui pongo attachmentes de color)
	//a la posicion 0 le asigno la salida 0 del shader (el attachment 0)

	// asignamos 1 al atachment zCam q estaba en pos 0:
	const GLenum buffs[4] = {	GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 }; //Hay un ejemplo de esto en el aula, mirarlo
	//el 0 no tiene attachment
	//al 1 le asigno el attachment 0
	//al 2 le asigno el attachment 1

	glDrawBuffers(4, buffs);
	//Por lo q cojo texturas y se las asigno a attachments y estas los cojo y los asigno a salidas del shader

	//Comprobamos q se ha configurado bn:
	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}

	//glBindFramebuffer(GL_FRAMEBUFFER, 0); //activa frame buffer 0 --> default frame buffer
	//si llamo a renderizar usa el default
}

void keyboardFunc(unsigned char key, int x, int y)
{
	static float angle = 0.1f;

	//glm::vec3 posLAux = glm::vec3(0.0);

	std::cout << "Se ha pulsado la tecla " << key << std::endl << std::endl;

	switch (key) {
		//Aumentar Motion blur
	case 'w':
		mb += 0.05f;
		mbcolor -= 0.05f;
		break;

		//Disminuir Motion blur
	case 's':
		mb -= 0.05f;
		mbcolor += 0.05f;
		break;

		//
	case 'd':
		dmax = dmax * 2;
		std::cout << "Distancia maxima: " << dmax << std::endl;
		break;
		//
	case 'a':
		dmax = dmax * 1 / 2; //disminuye sin llegar a 0
		std::cout << "Distancia maxima: " << dmax << std::endl;
		break;
		//
	case 'e':
		dfocal = dfocal + 1;
		std::cout << "Distancia focal: " << dfocal << std::endl;
		break;

		//
	case 'q':
		dfocal = dfocal - 1;
		std::cout << "Distancia focal: " << dfocal << std::endl;
		break;

		//
	case 'i':
		//enfocar
		maskF = float(1.0);
		mask = new float[25]{
			0.0f * maskF, 0.0f * maskF, 0.0f *  maskF, 0.0f *  maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, -1.0f * maskF, 0.0f *  maskF, 0.0f * maskF,
			0.0f * maskF, -1.0f *maskF, 5.0f *  maskF, -1.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, -1.0f * maskF, 0.0f *  maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, 0.0f *  maskF, 0.0f *  maskF, 0.0f * maskF };
		break;

	case 'k':
		//Realzar bordes
		maskF = float(1.0 / 2.0);
		mask = new float[25]{
			0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF,
			0.0f * maskF, -1.0f * maskF, 1.0f * maskF, 1.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF };
		break;

	case 'l':
		//Detectar bordes
		maskF = float(1.0 / 2.0);
		mask = new float[25]{
			0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, 1.0f * maskF, 0.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 1.0f * maskF, -4.0f * maskF, 1.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, 1.0f * maskF, 0.0f * maskF, 0.0f * maskF,
			0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF, 0.0f * maskF };
		break;

	
	case 'j':
		//posicionL += glm::vec3(-1.0f, 0.0, 0.0);
		break;

		//Mover luz hacia arriba
	case 'p':
		//posicionL += glm::vec3(0.0, 1.0f, 0.0);
		break;

		//Mover luz hacia abajo
	case 'o':
		//posicionL += glm::vec3(0.0, -1.0f, 0.0);
		break;

		//Intensidad de la luz
	case 'u':
		//if (intensidadL.x < 1) { //como x, y y z valen lo mismo, van a crecer y decrecer igual (al comparar x, sabemos que y y z valen lo mismo que x)
		//	//le ponemos un maximo de intensidad 15 por ejemplo
		//	intensidadL += glm::vec3(0.1f, 0.1f, 0.1f);
		//}
		break;

	case 'y':
		//if (intensidadL.x > 0) { //para que 
		//	intensidadL -= glm::vec3(0.1f, 0.1f, 0.1f);
		//}
		break;

	case 0:
		exit(0);
		break;
	}

	//posicionL = posLAux * posicionL;
	glutPostRedisplay();
}
void mouseFunc(int button, int state, int x, int y) {}