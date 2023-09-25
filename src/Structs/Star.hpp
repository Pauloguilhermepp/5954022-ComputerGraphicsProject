#ifndef STAR_HPP
#define STAR_HPP

#include <GL/glut.h>
#include "Coordinates.hpp"

// Define a structure for representing stars in the simulation
struct Star {
	GLfloat brightness;
	Coordinates pos;
};

#endif
