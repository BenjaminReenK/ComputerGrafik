#include "Planet.hpp"
#include "globalVars.hpp"

// Include GLM
// Mathematischer Stuff
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include GLEW
// Extensions (Shaders etc)
#include <GL/glew.h>

using namespace std;
using namespace glm;

Planet::Planet(float radius, float sunDistance, float timeOrbit, float timeRotation, float orbitalInclination, float obliquity, string name) {
	this->radius = radius;
	this->sunDistance = sunDistance;
	this->timeOrbit = timeOrbit;
	this->timeRotation = timeRotation;
	this->orbitalInclination = orbitalInclination;
	this->obliquity = obliquity;
	this->name = name;
	this->coords.x = 0;
	this->coords.y = 0;	
}

void Planet::calc(float step, mat4* model, float scaleFacDistance, float scaleFacPlanet) {	
	
	/**************    Calculation Part     ***************/
	
	// https://coolcodea.wordpress.com/2013/09/12/110-calculating-orbits-with-simple-trigonometry/
	// calculate the planet's angle toward the sun
	float angleToSun = (step * PI) / timeOrbit;
	// calculate x and y coordinates
	coords.x = sin(angleToSun) * sunDistance;
	coords.y = cos(angleToSun) * sunDistance;
	// calculate planet's rotation
	planetRotation = (step * 360) / timeRotation;

	/***************    Model Part     ******************/	
	
	// translation around sun
	vec3 transVec = vec3(coords.x * scaleFacDistance, coords.y * scaleFacDistance, 0.0);	
	currentPos = transVec;
	*model = translate(*model, transVec);
		
	// orbital incline
	*model = rotate(*model, orbitalInclination, vec3(1.0, 0.0, 0.0));

		// obliquity
	*model = rotate(*model, obliquity, vec3(1.0, 0.0, 0.0));

		// rotation on itself
	*model = rotate(*model, planetRotation, vec3(0.0, 0.0, 1.0)); 

	// correct texture mapping 
	*model = rotate(*model, 90.0f, vec3(1.0, 0.0, 0.0));
		
	// scale the planetsize
	// extra scaling for sun
	if (sunDistance == 0) {
		*model = scale(*model, vec3(0.2, 0.2, 0.2));
	} else { // scaling for all other planets
		*model = scale(*model, vec3(radius * scaleFacPlanet, radius * scaleFacPlanet, radius * scaleFacPlanet));
	}
	
}

void Planet::addMoon(Moon m) {
	this->moonList.push_back(m);
}

string Planet::getPlanetName() {
	return this->name;
}

float Planet::getRadius() {
	return this->radius;
}

float Planet::getSunDistance() {
	return this->sunDistance;
}

float Planet::getTimeRotation() {
	return this->timeRotation;
}
		
float Planet::getTimeOrbit() {
	return this->timeOrbit;
}

vector<Moon> Planet::getMoonList() {
	return this->moonList;
}

vec3 Planet::getCurrentPosition() {
	return this->currentPos;
}
