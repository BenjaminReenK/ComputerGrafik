// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <iostream>

// Include GLEW
// Extensions (Shaders etc)
#include <GL/glew.h>

// Include GLFW
// Fenster / Tastatur
#include <GLFW/glfw3.h>

// Include GLM
// Mathematischer Stuff
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Achtung, die OpenGL-Tutorials nutzen glfw 2.7, glfw kommt mit einem veränderten API schon in der Version 3 

// Befindet sich bei den OpenGL-Tutorials unter "common"
#include "shader.hpp"

// Wuerfel und Kugel
#include "objects.hpp"

// Objectloader
#include "objloader.hpp"
// Textureloader
#include "texture.hpp"
// custom headers
#include "Planet.hpp"
#include "moon.hpp"
#include "globalVars.hpp"
#include "camera.hpp"
#include "planetinfo.hpp"
#include "text2D.hpp"
#include "vboindexer.hpp"
#include "SOIL.h"

using namespace glm;
using namespace std;

/****************    Global Variables    ****************/
// Diese Drei Matrizen global (Singleton-Muster), damit sie jederzeit modifiziert und
// an die Grafikkarte geschickt werden koennen
//Transformationsmatrizen

glm::mat4 projection; // Kamera -> Projektionsfläche
glm::mat4 view;  // Welt -> Kamera
glm::mat4 model; //Modelkoordinaten / Objekt -> Weltkoordinaten
glm::mat4 saveModel;
// some matrices for bumpmapping
glm::mat4 ModelViewMatrix;
glm::mat3 ModelView3x3Matrix;
glm::mat4 MVP;

GLuint programID; // Shader ProgramID
GLuint bumpShader; // Shader Bumpmapping
GLuint skyShader; // Shader Skybox

// planet movement
float currentStep = 0.0;
float stepDistance = 0.00005;

// scale factors
float scaleFactorDistance = 0.00000001;
float scaleFactorPlanet = 0.00001;

// custom camera class
Camera camera(vec3(0.0f, -5.0f, -5.0f), vec3(0.0f, 0.0f, -1.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

// mouse vars
bool firstMouse = true;
GLfloat lastMouseX = 0.0f, lastMouseY = 0.0f;

// boolean array representing the keys pressed/released
// allows us so press more then one key and move diagonal 
bool keys[1024];

// vector holding all our planets, array with custom Class Planet didn't work, so it's a vector now..
vector<Planet> planetList;

// currently selected planet (using numpad keys)
int currentPlanet = -1;

// Textures
GLuint texture_sun;
GLuint texture_merkur;
GLuint texture_venus;
GLuint texture_earth;
GLuint texture_mars;
GLuint texture_jupiter;
GLuint texture_saturn;
GLuint texture_uranus;
GLuint texture_neptun;
GLuint texture_moon;

GLuint texture_merkur_normal;
GLuint texture_venus_normal;
GLuint texture_earth_normal;
GLuint texture_mars_normal;
GLuint texture_jupiter_normal;
GLuint texture_saturn_normal;
GLuint texture_uranus_normal;
GLuint texture_neptun_normal;
GLuint texture_moon_normal;

// bump mapping stuff
GLuint DiffuseTexture;
GLuint NormalTexture;
GLuint SpecularTexture;

GLuint DiffuseTextureID;
GLuint NormalTextureID;
GLuint SpecularTextureID;

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint ModelView3x3MatrixID;

// sphere object vectors
vector<vec3> vertices;
vector<vec2> uvs;  // 2D Vektor -> für Oberflächenstruktur
vector<vec3> normals; // 3D Vektor -> Oberflächennormalen
GLuint VertexArrayIDPlanet = 0;

// sun is special, inverted normals
vector<vec3> verticesSun;
vector<vec2> uvsSun;  // 2D Vektor -> für Oberflächenstruktur
vector<vec3> normalsSun; // 3D Vektor -> Oberflächennormalen
GLuint VertexArrayIDSun = 0;

// vectors for bumpmapping
vector<glm::vec3> tangents;
vector<glm::vec3> bitangents;

vector<unsigned short> indices;
vector<glm::vec3> indexed_vertices;
vector<glm::vec2> indexed_uvs;
vector<glm::vec3> indexed_normals;
vector<glm::vec3> indexed_tangents;
vector<glm::vec3> indexed_bitangents;

// bumpmap buffers
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
GLuint tangentbuffer;
GLuint bitangentbuffer;
GLuint elementbuffer;
GLuint LightID;

/****************   Functions Start   *****************/

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//printf("Key: %d %d\n", key, action);

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}

	if (action == GLFW_RELEASE) {
		return; // ignore release events
	}

	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
		// "+" to increase simulation speed
	case GLFW_KEY_L:
		stepDistance += 0.00005;
		break;
		// "-" to decrease simulation speed
	case GLFW_KEY_O:
		stepDistance -= 0.00005;
		break;
		// increase scale factor for distance
	case GLFW_KEY_F1:
		scaleFactorDistance *= 1.1;
		break;
		// decrease scale factor for distance
	case GLFW_KEY_F2:
		scaleFactorDistance /= 1.1;
		break;
		// increase scale factor for planet size
	case GLFW_KEY_F3:
		scaleFactorPlanet *= 1.1;
		break;
		// decrease scale factor for planet
	case GLFW_KEY_F4:
		scaleFactorPlanet /= 1.1;
		break;

	// WASDEQ in doCameraMovement()

		// for closing planet's informations
	case GLFW_KEY_0:
	case GLFW_KEY_KP_0:
		currentPlanet = -1;
		break;
		// jump to sun
	case GLFW_KEY_1:
	case GLFW_KEY_KP_1:
		currentPlanet = 0;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to merkur
	case GLFW_KEY_2:
	case GLFW_KEY_KP_2:
		currentPlanet = 1;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to venus
	case GLFW_KEY_3:
	case GLFW_KEY_KP_3:
		currentPlanet = 2;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to earth
	case GLFW_KEY_4:
	case GLFW_KEY_KP_4:
		currentPlanet = 3;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to mars
	case GLFW_KEY_5:
	case GLFW_KEY_KP_5:
		currentPlanet = 4;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to jupiter
	case GLFW_KEY_6:
	case GLFW_KEY_KP_6:
		currentPlanet = 5;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to saturn
	case GLFW_KEY_7:
	case GLFW_KEY_KP_7:
		currentPlanet = 6;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to uranus
	case GLFW_KEY_8:
	case GLFW_KEY_KP_8:
		currentPlanet = 7;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;
		// jump to neptun
	case GLFW_KEY_9:
	case GLFW_KEY_KP_9:
		currentPlanet = 8;
		camera.AnimateTo(planetList[currentPlanet].getCurrentPosition());
		break;	
	default:
		break;
	}
}

void computeTangentBasis(
	// inputs
	vector<glm::vec3> & vertices,
	vector<glm::vec2> & uvs,
	vector<glm::vec3> & normals,
	// outputs
	vector<glm::vec3> & tangents,
	vector<glm::vec3> & bitangents
	){
		// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
		for (unsigned int i=0; i<vertices.size(); i+=3 ){

			// Shortcuts for vertices
			glm::vec3 & v0 = vertices[i+0];
			glm::vec3 & v1 = vertices[i+1];
			glm::vec3 & v2 = vertices[i+2];

			// Shortcuts for UVs
			glm::vec2 & uv0 = uvs[i+0];
			glm::vec2 & uv1 = uvs[i+1];
			glm::vec2 & uv2 = uvs[i+2];

			// Edges of the triangle : postion delta
			glm::vec3 deltaPos1 = v1-v0;
			glm::vec3 deltaPos2 = v2-v0;

			// UV delta
			glm::vec2 deltaUV1 = uv1-uv0;
			glm::vec2 deltaUV2 = uv2-uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

			// Set the same tangent for all three vertices of the triangle.
			// They will be merged later, in vboindexer.cpp
			tangents.push_back(tangent);
			tangents.push_back(tangent);
			tangents.push_back(tangent);

			// Same thing for binormals
			bitangents.push_back(bitangent);
			bitangents.push_back(bitangent);
			bitangents.push_back(bitangent);

		}

		for (unsigned int i=0; i<vertices.size(); i+=1 )
		{
			glm::vec3 & n = normals[i];
			glm::vec3 & t = tangents[i];
			glm::vec3 & b = bitangents[i];

			// Gram-Schmidt orthogonalize
			t = glm::normalize(t - n * glm::dot(n, t));

			// Calculate handedness
			if (glm::dot(glm::cross(n, t), b) < 0.0f){
				t = t * -1.0f;
			}
		}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	// printf("Mouse: %f %f\n", xpos, ypos);

	if (firstMouse) { // this bool variable is initially set to true
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastMouseX;
	GLfloat yoffset = lastMouseY - ypos; // Reversed since y-coordinates range from bottom to top
	lastMouseX = xpos;
	lastMouseY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// printf("Scroll: %f %f\n", xoffset, yoffset);
	camera.ProcessMouseScroll(yoffset);
}

void sendMVP() {
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = projection * view * model; 
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform, konstant fuer alle Eckpunkte
	// schreibt Transformationsmatrix MVP in die Variable "MVP" vom Shader
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVP[0][0]); //gibt Handle zum Shader
	// schreibt Viewmatrix View in die Variable "V" vom Shader
	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &model[0][0]);
	// schreibt Modelmatrix Model in die Variable "M" vom Shader
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &view[0][0]);
}

// sepereate MVP function for bumpmapping, didn't want to change the original one...
void sendMVPBump() {

	MatrixID = glGetUniformLocation(bumpShader, "MVP");
	ViewMatrixID = glGetUniformLocation(bumpShader, "V");
	ModelMatrixID = glGetUniformLocation(bumpShader, "M");
	ModelView3x3MatrixID = glGetUniformLocation(bumpShader, "MV3x3");

	ModelViewMatrix = view * model;
	ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
	MVP = projection * view * model;

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);
}

void prepBumpBuffers() {
	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
	// Bind our diffuse texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DiffuseTexture);
	// Set our "DiffuseTextureSampler" sampler to user Texture Unit 0
	glUniform1i(DiffuseTextureID, 0);

	// Bind our normal texture in Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalTexture);
	// Set our "Normal	TextureSampler" sampler to user Texture Unit 0
	glUniform1i(NormalTextureID, 1);

	// Bind our normal texture in Texture Unit 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, SpecularTexture);
	// Set our "Normal	TextureSampler" sampler to user Texture Unit 0
	glUniform1i(SpecularTextureID, 2);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 4th attribute buffer : tangents
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
	glVertexAttribPointer(
		3,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 5th attribute buffer : bitangents
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
	glVertexAttribPointer(
		4,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		indices.size(),    // count
		GL_UNSIGNED_SHORT, // type
		(void*)0           // element array buffer offset
		);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
}

// using Sphere from a object file, because drawSphere()
// and gluSphere aren't working with textures, don't know why..
void loadSphere() {
	// load sphere object from file
	bool loaded = loadOBJ("res/sphere.obj", vertices, uvs, normals);

	if (loaded) {

		glGenVertexArrays(1, &VertexArrayIDPlanet); // VertexArray Objekt, notiert sich alles was mit Buffern etc. gemacht wird
		glBindVertexArray(VertexArrayIDPlanet); // VertexArray Object aktivieren

		/************  Vertices  ****************/

		// Buffer worin die Vertizes gespeichert werden sollen um sie zur Graka zur schicken
		GLuint vBuffer;
		// Buffer eine ID zuweisen und speichern
		glGenBuffers(1, &vBuffer);
		// Buffer aktivieren
		glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
		// Daten in Buffer schreiben
		glBufferData(GL_ARRAY_BUFFER,
			vertices.size() /* Anzahl Elemente bzw. Länge des Vektor */ * sizeof(glm::vec3) /* Bytegröße eines Vektors */,
			&vertices[0] /* Adresse des 1. Elementes vom Vektor */,
			GL_STATIC_DRAW); // Daten werden nicht mehr geändert zur Laufzeit
		// Kein Disable führen
		glEnableVertexAttribArray(0); 
		// VertexShader Attribut "0" setzen
		glVertexAttribPointer(0, /* 1. Element, 0 entspricht der input variable 0 im vertexshader */
			3, /* 3 Werte */
			GL_FLOAT, /* Floats */
			GL_FALSE, /* nicht normalisiert */
			0, /* elemente stehen hintereinander im Speicher */
			(void*) 0); /* Array Buffer Offset */

		/************  Oberflächennormalen  ****************/

		// Buffer für die Oberflächennormalen
		GLuint nBuffer;
		// Buffer eine ID zuweisen und speichern
		glGenBuffers(1, &nBuffer);
		// Buffer aktivieren
		glBindBuffer(GL_ARRAY_BUFFER, nBuffer);
		// Daten in Buffer schreiben
		glBufferData(GL_ARRAY_BUFFER, 
			normals.size() * sizeof(glm::vec3),
			&normals[0],
			GL_STATIC_DRAW);
		// 2 analog zum 1. Parameter von glVertexAttribPointer -> 3. Element
		glEnableVertexAttribArray(2);
		// VertexShader Attribut "2" setzen
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		/************  Oberflächenstrukturen  ****************/

		// Buffer für die Oberflächenstruktur
		GLuint uvBuf;
		// Buffer eine ID zuweisen und speichern
		glGenBuffers(1, &uvBuf);
		// Buffer aktivieren
		glBindBuffer(GL_ARRAY_BUFFER, uvBuf);
		// Daten in Buffer schreiben
		glBufferData(GL_ARRAY_BUFFER,
			uvs.size() * sizeof(glm::vec3),
			&uvs[0],
			GL_STATIC_DRAW);
		// 2. Parameter mit Index 1 
		glEnableVertexAttribArray(1);
		// VertexShader mit Index 1 setzen
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
		// VertexArray Object deaktivieren
		glBindVertexArray(0);
	}
}

void loadSphereSun() {
		// load sphere object from file
	bool loaded = loadOBJ("res/sphere.obj", verticesSun, uvsSun, normalsSun);

	// invert normals
	for (int i = 0; i < normalsSun.size(); i++){
		normalsSun[i] = -normalsSun[i];
	}

	if (loaded) {
		glGenVertexArrays(1, &VertexArrayIDSun); // VertexArray Objekt, notiert sich alles was mit Buffern etc. gemacht wird
		glBindVertexArray(VertexArrayIDSun); // VertexArray Object aktivieren

		/************  Vertices  ****************/

		// Buffer worin die Vertizes gespeichert werden sollen um sie zur Graka zur schicken
		GLuint vBuffer;
		// Buffer eine ID zuweisen und speichern
		glGenBuffers(1, &vBuffer);
		// Buffer aktivieren
		glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
		// Daten in Buffer schreiben
		glBufferData(GL_ARRAY_BUFFER,
			verticesSun.size() /* Anzahl Elemente bzw. Länge des Vektor */ * sizeof(glm::vec3) /* Bytegröße eines Vektors */,
			&verticesSun[0] /* Adresse des 1. Elementes vom Vektor */,
			GL_STATIC_DRAW); // Daten werden nicht mehr geändert zur Laufzeit
		// Kein Disable führen
		glEnableVertexAttribArray(0); 
		// VertexShader Attribut "0" setzen
		glVertexAttribPointer(0, /* 1. Element, 0 entspricht der input variable 0 im vertexshader */
			3, /* 3 Werte */
			GL_FLOAT, /* Floats */
			GL_FALSE, /* nicht normalisiert */
			0, /* elemente stehen hintereinander im Speicher */
			(void*) 0); /* Array Buffer Offset */

		/************  Oberflächennormalen  ****************/

		// Buffer für die Oberflächennormalen
		GLuint nBuffer;
		// Buffer eine ID zuweisen und speichern
		glGenBuffers(1, &nBuffer);
		// Buffer aktivieren
		glBindBuffer(GL_ARRAY_BUFFER, nBuffer);
		// Daten in Buffer schreiben
		glBufferData(GL_ARRAY_BUFFER, 
			normalsSun.size() * sizeof(glm::vec3),
			&normalsSun[0],
			GL_STATIC_DRAW);
		// 2 analog zum 1. Parameter von glVertexAttribPointer -> 3. Element
		glEnableVertexAttribArray(2);
		// VertexShader Attribut "2" setzen
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		/************  Oberflächenstrukturen  ****************/

		// Buffer für die Oberflächenstruktur
		GLuint uvBuf;
		// Buffer eine ID zuweisen und speichern
		glGenBuffers(1, &uvBuf);
		// Buffer aktivieren
		glBindBuffer(GL_ARRAY_BUFFER, uvBuf);
		// Daten in Buffer schreiben
		glBufferData(GL_ARRAY_BUFFER,
			uvsSun.size() * sizeof(glm::vec3),
			&uvsSun[0],
			GL_STATIC_DRAW);
		// 2. Parameter mit Index 1 
		glEnableVertexAttribArray(1);
		// VertexShader mit Index 1 setzen
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
		// VertexArray Object deaktivieren
		glBindVertexArray(0);
	}
}

// used for planets
void drawSphereObject() {
	// activate VertexArray
	glBindVertexArray(VertexArrayIDPlanet);
	// draw vertex object
	glDrawArrays(GL_TRIANGLES /* Dreiecke zeichnen */, 0, vertices.size());
	// disable VertexArray
	glBindVertexArray(0);
}

void drawSphereSunObject() {	
	glBindVertexArray(VertexArrayIDSun); // activate VertexArray	
	glDrawArrays(GL_TRIANGLES /* Dreiecke zeichnen */, 0, vertices.size()); // draw vertex object	
	glBindVertexArray(0); // disable VertexArray
}

void init(){
	// load textures from file ... Source: http://planetpixelemporium.com/planets.html
	texture_sun = loadBMP_custom("res/sun.bmp");
	texture_merkur = loadBMP_custom("res/mercury.bmp");
	texture_venus = loadBMP_custom("res/venus.bmp");
	texture_earth = loadBMP_custom("res/earth2.bmp");
	texture_mars = loadBMP_custom("res/mars.bmp");
	texture_jupiter = loadBMP_custom("res/jupiter.bmp");
	texture_saturn = loadBMP_custom("res/saturn.bmp");
	texture_uranus = loadBMP_custom("res/uranus.bmp");
	texture_neptun = loadBMP_custom("res/neptun.bmp");
	texture_moon = loadBMP_custom("res/moon.bmp");
	// load normal maps
	texture_merkur_normal = loadBMP_custom("res/mercury_normal.bmp");
	texture_venus_normal = loadBMP_custom("res/venus_normal.bmp");
	texture_earth_normal = loadBMP_custom("res/earth2_normal.bmp");
	texture_mars_normal = loadBMP_custom("res/mars_normal.bmp");
	texture_jupiter_normal = loadBMP_custom("res/jupiter_normal.bmp");
	texture_saturn_normal = loadBMP_custom("res/saturn_normal.bmp");
	texture_uranus_normal = loadBMP_custom("res/uranus_normal.bmp");
	texture_neptun_normal = loadBMP_custom("res/neptun_normal.bmp");
	texture_moon_normal = loadBMP_custom("res/moon_normal.bmp");

	// Initiate Planetlist, Values from http://astrokramkiste.de/planeten-tabelle
	// https://en.wikipedia.org/wiki/Orbital_inclination
	// https://en.wikipedia.org/wiki/Axial_tilt
	// Pluto isn't included, because it's not a planet anymore...
	Planet sonne = Planet(695500.0, 0.0, 1.0, 500.0f, 0.0, 7.25, "Sonne");
	Planet merkur = Planet(2439.5, 58000000.0, 88.0, 58.67f, 6.34, 0.03, "Merkur");
	Planet venus = Planet(6051.5, 108000000.0, 225.0, 243.0f, 3.39, 177.36, "Venus");
	Planet erde = Planet(6367.5, 150000000.0, 365.0, 1.0f, 0.0, 23.44, "Erde");
	Planet mars = Planet(3386.0, 228000000.0, 687.0, 1.04f, 1.85, 25.19, "Mars");
	Planet jupiter = Planet(69173.0, 778000000.0, 4329.0, 0.42f, 1.31, 3.13, "Jupiter");
	Planet saturn = Planet(57316.0, 1433000000.0, 10751.0, 0.45f, 2.49, 26.73, "Saturn");
	Planet uranus = Planet(25266.5, 2872000000.0, 30664.0, 0.71f, 0.77, 97.77, "Uranus");
	Planet neptun = Planet(24552.5, 4495000000.0, 60148.0, 0.67f, 1.77, 28.32, "Neptun");

	// moons, just for earth to show how its working
	Moon earthMoon = Moon(17400.0, 200000000.0, 27.3, 27.3, 5.14, 25.19);
	erde.addMoon(earthMoon);

	// add planet to planetlist for later usage
	// don't add moon, because it rotates around the earth, not the sun
	planetList.push_back(sonne);
	planetList.push_back(merkur);
	planetList.push_back(venus);
	planetList.push_back(erde);
	planetList.push_back(mars);
	planetList.push_back(jupiter);
	planetList.push_back(saturn);
	planetList.push_back(uranus);
	planetList.push_back(neptun);

	loadSphere();
	loadSphereSun();

	// bumpmap stuff
	// bumpmap textures

	DiffuseTextureID  = glGetUniformLocation(bumpShader, "DiffuseTextureSampler");
	NormalTextureID  = glGetUniformLocation(bumpShader, "NormalTextureSampler");
	SpecularTextureID  = glGetUniformLocation(bumpShader, "SpecularTextureSampler");

	//calculate tangents
	computeTangentBasis(
		vertices, uvs, normals, // input
		tangents, bitangents    // output
		);
	// index our vbo
	indexVBO_TBN(
		vertices, uvs, normals, tangents, bitangents, 
		indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents
		);

	// Load it into a VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &tangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof(glm::vec3), &indexed_tangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &bitangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof(glm::vec3), &indexed_bitangents[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
	
}

void initSkybox(GLuint* vao) {
	float sz = 110.0f;
	GLfloat skyboxVertices[] = {
		// Positions          
		-sz,  sz, -sz,
		-sz, -sz, -sz,
		sz, -sz, -sz,
		sz, -sz, -sz,
		sz,  sz, -sz,
		-sz,  sz, -sz,

		-sz, -sz,  sz,
		-sz, -sz, -sz,
		-sz,  sz, -sz,
		-sz,  sz, -sz,
		-sz,  sz,  sz,
		-sz, -sz,  sz,

		sz, -sz, -sz,
		sz, -sz,  sz,
		sz,  sz,  sz,
		sz,  sz,  sz,
		sz,  sz, -sz,
		sz, -sz, -sz,

		-sz, -sz,  sz,
		-sz,  sz,  sz,
		sz,  sz,  sz,
		sz,  sz,  sz,
		sz, -sz,  sz,
		-sz, -sz,  sz,

		-sz,  sz, -sz,
		sz,  sz, -sz,
		sz,  sz,  sz,
		sz,  sz,  sz,
		-sz,  sz,  sz,
		-sz,  sz, -sz,

		-sz, -sz, -sz,
		-sz, -sz,  sz,
		sz, -sz, -sz,
		sz, -sz, -sz,
		-sz, -sz,  sz,
		sz, -sz,  sz
	};

	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	*vao = skyboxVAO;
}

// activate matching bumpmap textures
void setBumpmapTexture(int planetNum) {
	switch(planetNum) {
	case 1:
		DiffuseTexture = texture_merkur;
		NormalTexture = texture_merkur_normal;
		break;
	case 2:
		DiffuseTexture = texture_venus;
		NormalTexture = texture_venus_normal;
		break;
	case 3:
		DiffuseTexture = texture_earth;
		NormalTexture = texture_earth_normal;
		break;
	case 4:
		DiffuseTexture = texture_mars;
		NormalTexture = texture_mars_normal;
		break;
	case 5:
		DiffuseTexture = texture_jupiter;
		NormalTexture = texture_jupiter_normal;
		break;
	case 6:
		DiffuseTexture = texture_saturn;
		NormalTexture = texture_saturn_normal;
		break;
	case 7:
		DiffuseTexture = texture_uranus;
		NormalTexture = texture_uranus_normal;
		break;
	case 8:
		DiffuseTexture = texture_neptun;
		NormalTexture = texture_neptun_normal;
		break;
	case 9:
		DiffuseTexture = texture_moon;
		NormalTexture = texture_moon_normal;
		break;
	default:
		DiffuseTexture = texture_earth;
		NormalTexture = texture_earth_normal;
		break;
	}
}

void drawPlanets() {
	
	for (int i = 0; i < planetList.size(); i++) {
		// activate texture 0 and rebind it each cycle, so texture0 from bumpmapping doesn't get used on every planet
		glActiveTexture(GL_TEXTURE0);
		// bind matching planet texture
		switch (i) {
		case 0:
			glBindTexture(GL_TEXTURE_2D, texture_sun);
			break;
		case 1:
			glBindTexture(GL_TEXTURE_2D, texture_merkur);
			break;
		case 2:
			glBindTexture(GL_TEXTURE_2D, texture_venus);
			break;
		case 3:
			glBindTexture(GL_TEXTURE_2D, texture_earth);
			break;
		case 4:
			glBindTexture(GL_TEXTURE_2D, texture_mars);
			break;
		case 5:
			glBindTexture(GL_TEXTURE_2D, texture_jupiter);
			break;
		case 6:
			glBindTexture(GL_TEXTURE_2D, texture_saturn);
			break;
		case 7:
			glBindTexture(GL_TEXTURE_2D, texture_uranus);
			break;
		case 8:
			glBindTexture(GL_TEXTURE_2D, texture_neptun);
			break;
		default:
			break;
		}
		// only draw sun without bumpmapping, don't know why it is not working..
		if (i == 0) { 
			// save model matrix
			saveModel = model;
			// calculate planet position, spinning ...
			planetList[i].calc(currentStep, &model, scaleFactorDistance, scaleFactorPlanet);
			sendMVP();
			// draw the sphere with new calculated model matrix
			drawSphereSunObject();			
			// reset model matrix
			model = saveModel;
		}
		// use bumpmapping for every other planet
		else {
			// use bumpmap shader
			glUseProgram(bumpShader); 
			// save model matrix
			saveModel = model;
			// calculate planet position, spinning ...
			planetList[i].calc(currentStep, &model, scaleFactorDistance, scaleFactorPlanet);
			setBumpmapTexture(i);
			sendMVPBump();
			prepBumpBuffers();
			// draw the sphere with new calculated model matrix
			drawSphereObject();
			// moons
			vector<Moon> mList = planetList[i].getMoonList();
			// if the planet has a moon
			if (mList.size() > 0) {
				for (int i = 0; i < mList.size(); i++) {
					// save model state after planet translation, so we can draw multiple moons
					mat4 moonmodel = model;
					Moon m = mList[i];
					// calculate the moon position
					m.calc(currentStep, &model, scaleFactorDistance, scaleFactorPlanet);
					setBumpmapTexture(9);
					sendMVPBump();
					prepBumpBuffers();
					// draw the sphere with new calculated model matrix
					drawSphereObject();
					// reset model
					model = moonmodel;
				}
			}
			// reset model matrix
			model = saveModel;
		}
	}
	// unbind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void doCameraMovement() {
	if (keys[GLFW_KEY_Q]) { // up
		camera.ProcessKeyboard(UP);
	}

	if (keys[GLFW_KEY_E]) { // down
		camera.ProcessKeyboard(DOWN);
	}

	if (keys[GLFW_KEY_W]) { // backwards
		camera.ProcessKeyboard(FORWARD);
	}

	if(keys[GLFW_KEY_S]) {// towards position
		camera.ProcessKeyboard(BACKWARD);
	}

	if(keys[GLFW_KEY_A]) { // left
		camera.ProcessKeyboard(LEFT);
	}

	if(keys[GLFW_KEY_D]) { // right
		camera.ProcessKeyboard(RIGHT);
	}

}

// Loads a cubemap texture from 6 individual texture faces
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width,height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for(GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

int main(void) {
	if (!glfwInit()) { // Initialise GLFW
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Fehler werden auf stderr ausgegeben, s. o.
	glfwSetErrorCallback(error_callback);

	// use 8x MSSA --> http://www.learnopengl.com/#!Advanced-OpenGL/Anti-aliasing
	glfwWindowHint(GLFW_SAMPLES, 8);

	// http://www.glfw.org/docs/latest/window.html#window_windowed_full_screen
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());		
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	// create windowed full screen
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, TITLE, NULL, NULL);
	/*
	// Open a window and create its OpenGL context
	// glfwWindowHint vorher aufrufen, um erforderliche Resourcen festzulegen
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, // Breite
	SCREEN_HEIGHT,  // Hoehe
	TITLE, // Ueberschrift
	NULL,  // windowed mode
	NULL); // shared windoe
	*/
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Make the window's context current (wird nicht automatisch gemacht)
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	// GLEW ermöglicht Zugriff auf OpenGL-API > 1.1
	glewExperimental = true; // Needed for core profile

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_MULTISAMPLE);  
	
	// Auf Keyboard-Events reagieren
	glfwSetKeyCallback(window, key_callback);

	// Auf Mouse-Events reagieren
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
	glfwSetCursorPosCallback(window, mouse_callback); 

	// Auf Scroll-Events reagieren
	glfwSetScrollCallback(window, scroll_callback); 

	// Create and compile our GLSL program from the shaders
	// Vertex- und FragmentShader laden
	programID = LoadShaders("shader/StandardShading.vertexshader", "shader/StandardShading.fragmentshader"); // für Standard durchsichtigen Würfel
	bumpShader= LoadShaders("shader/NormalMapping.vertexshader", "shader/NormalMapping.fragmentshader");
	skyShader = LoadShaders("shader/Skybox.vertexshader", "shader/Skybox.fragmentshader");
	
	init();
	initText2D("Holstein.DDS");
	GLuint skyboxVAO;
	initSkybox(&skyboxVAO);	

	vector<const GLchar*> faces;
	faces.push_back("res/sky/starfield_right.jpg");
	faces.push_back("res/sky/starfield_left.jpg");
	faces.push_back("res/sky/starfield_top.jpg");
	faces.push_back("res/sky/starfield_bottom.jpg");
	faces.push_back("res/sky/starfield_back.jpg");
	faces.push_back("res/sky/starfield_front.jpg");
	GLuint cubemapTexture = loadCubemap(faces);

	// Eventloop
	while (!glfwWindowShouldClose(window)) {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
		glClearColor(0.6f, 0.6f, 0.8f, 0.0f);

		// Draw skybox first
		glDepthMask(GL_FALSE);// Remember to turn depth writing off
		glUseProgram(skyShader);

		projection = perspective(camera.GetFoV(), (float) mode->width / (float) mode->height, 0.1f, 250.0f);  

		// Camera matrix
		doCameraMovement();		
		view = camera.GetViewMatrix();

		glUniformMatrix4fv(glGetUniformLocation(skyShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(skyShader, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
				
		model = mat4(1.0f); // Model matrix : an identity matrix (model will be at the origin)

		sendMVP();

		currentStep -= stepDistance;
		
		glUseProgram(programID); // Shader auch benutzen

		glm::vec3 lightPos = glm::vec3(0,0,0); // Lichtposition setzen
		// Lichtposition an Shader senden -> "LightPosition_worldspace" im Shader setzen
		glUniform3f(glGetUniformLocation(programID, "LightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);
		// lichtposi an bumpshader senden
		glUniform3f(glGetUniformLocation(bumpShader, "LightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);

		// drawing
		drawPlanets();

		// draw planet information, if a planet is selected via numpad
		if (currentPlanet != -1) {
			drawPlanetInformations(mode->width, mode->height, planetList[currentPlanet]);
		}
				
		glfwSwapBuffers(window); // Swap buffers

		glfwPollEvents(); // Poll for and process events 
	}

	cleanupText2D();

	// Shader freigeben
	glDeleteProgram(programID);
	glDeleteProgram(bumpShader);
	glDeleteProgram(skyShader);

	glfwTerminate(); // Close OpenGL window and terminate GLFW

	return 0;
}
