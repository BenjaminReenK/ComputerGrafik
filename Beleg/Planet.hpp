#ifndef PLANET_HPP
#define PLANET_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "moon.hpp"

using namespace std;
using namespace glm;

class Planet {
	
	public:
		// Constructor
		Planet(float radius, float sunDistance, float timeOrbit, float timeRotation, float orbitalInclination, float obliquity, string name);
		// calculate new planet position and edits model matrix
		void calc(float step, mat4* model, float scaleFacDistance, float scaleFacPlanet);
		// add moon to moonlist
		void addMoon(Moon m);
		// get planet name
		string getPlanetName();
		// get planet radius
		float getRadius();
		// get distance to sun
		float getSunDistance();
		// get time rotation
		float getTimeRotation();
		// get time orbit
		float getTimeOrbit();
		// moonlist
		vector<Moon> getMoonList(); 
		// return current planet position
		vec3 getCurrentPosition();
		
	private:
		// Planet radius in km
		float radius;
		// distance to the sun in Mio. km
		float sunDistance;
		// amount of time needed for a full spin of the planet, in days
		float timeRotation;
		// amount of time needed for a full orbit travel around the sun, in days
		float timeOrbit;
		// vector holding planet coordinates
		vec2 coords;
		// planet name
		string name;
		// used for the planet's own rotation
		float planetRotation;
		// angle between the plane of the orbit of the planet and the ecliptic
		float orbitalInclination;
		// angle between an object's rotational axis and its orbital axis
		// or, equivalently, the angle between its equatorial plane and orbital plane
		float obliquity;
		// moonlist
		vector<Moon> moonList;
		// vector holding current planet position
		vec3 currentPos;		
};

#endif