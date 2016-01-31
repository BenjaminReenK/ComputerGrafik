#ifndef MOON_HPP
#define MOON_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>

using namespace glm;

class Moon {
	public:
		Moon(float radius, float planetDistance, float timeOrbit, float timeRotation, float orbitalInclination, float obliquity);
		// calculate new planet position and edits model matrix
		void calc(float step, mat4* model, float scaleFacDistance, float scaleFacPlanet);
	private:
		// moon radius in km
		float radius;
		// distance to the planet in Mio. km
		float planetDistance;
		// amount of time needed for a full spin of the moon, in days
		float timeRotation;
		// amount of time needed for a full orbit travel around the planet, in days
		float timeOrbit;
		// vector holding moon coordinates
		vec2 coords;
		// used for the moon's own rotation
		float planetRotation;
		// angle between the plane of the orbit of the planet and the ecliptic
		float orbitalInclination;
		// angle between an object's rotational axis and its orbital axis
		// or, equivalently, the angle between its equatorial plane and orbital plane
		float obliquity;
};


#endif