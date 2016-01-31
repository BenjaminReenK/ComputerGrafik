#include "Planet.hpp"
#include "text2D.hpp"


using namespace std;

string format_string(string s) {
	 // find decimal seperator
    int posDecSeperator = s.find('.');
    // 2 digits after comma
    string result = s.substr(0, posDecSeperator+3);
	return result;
}

void drawPlanetInformations(int screenwidth, int screenheight, Planet planet) {
	int yOffset = 20;
	int count = 0;
	string tmp;
	// planet name
	string name = "Name: ";
	
	name.append(planet.getPlanetName());
	const char *p = name.c_str();
	printText2D(p, screenwidth * 0.21, screenheight * 0.1, 15);
	count++;
	// radius
	string radius = "Radius: ";
	tmp = format_string(to_string(planet.getRadius()));
	radius.append(tmp);
	p = radius.c_str();
	printText2D(p, screenwidth * 0.21, (screenheight * 0.1) - (yOffset * count), 15);
	count++;
	// sundistance
	string distance = "Sundistance: ";
	tmp = format_string(to_string(planet.getSunDistance()));
	distance.append(tmp);
	p = distance.c_str();
	printText2D(p, screenwidth * 0.21, (screenheight * 0.1) - (yOffset * count), 15);
	count++;
	// rotation time
	string rotationtime = "Rotation Time: ";
	tmp = format_string(to_string(planet.getTimeRotation()));
	rotationtime.append(tmp);
	p = rotationtime.c_str();
	printText2D(p, screenwidth * 0.21, (screenheight * 0.1) - (yOffset * count), 15);
	count++;
	// orbit time
	string orbittime = "Orbit Time: ";
	tmp = format_string(to_string(planet.getTimeOrbit()));
	orbittime.append(tmp);
	p = orbittime.c_str();
	printText2D(p, screenwidth * 0.21, (screenheight * 0.1) - (yOffset * count), 15);
	count++;
}