#include "moon.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "globalVars.hpp"

using namespace glm;

Moon::Moon(float radius, float planetDistance, float timeOrbit, float timeRotation, float orbitalInclination, float obliquity) {
	this->radius = radius;
	this->planetDistance = planetDistance;
	this->timeOrbit = timeOrbit;
	this->timeRotation = timeRotation;
	this->orbitalInclination;
	this->obliquity = obliquity;
	this->coords.x = 0;
	this->coords.y = 0;
}

void Moon::calc(float step, mat4* model, float scaleFacDistance, float scaleFacPlanet) {

	/**************    Calculation Part     ***************/
	
	// https://coolcodea.wordpress.com/2013/09/12/110-calculating-orbits-with-simple-trigonometry/
	// calculate the planet's angle toward the sun
	float angleToPlanet = (step * PI) / timeOrbit;
	// calculate x and y coordinates
	coords.x = sin(angleToPlanet) * planetDistance;
	coords.y = cos(angleToPlanet) * planetDistance;
	// calculate planet's rotation
	planetRotation = (step * 360) / timeRotation;

	/***************    Model Part     ******************/	
	
	// undo the rotation used for texture correction, so the moon translates correctly
	*model = rotate(*model, -90.0f, vec3(1.0, 0.0, 0.0));

	// translation around planet
	vec3 transVec = vec3(coords.x * scaleFacDistance, coords.y * scaleFacDistance, 0.0);	
	*model = translate(*model, transVec);	
	
	// orbital incline
	*model = rotate(*model, orbitalInclination, vec3(1.0, 0.0, 0.0));

	// obliquity
	*model = rotate(*model, obliquity, vec3(1.0, 0.0, 0.0));

	// rotation on itself
	*model = rotate(*model, planetRotation, vec3(0.0, 0.0, 1.0)); 	

	*model = scale(*model, vec3(radius * scaleFacPlanet, radius * scaleFacPlanet, radius * scaleFacPlanet));
}
