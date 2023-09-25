#ifndef STAR_HPP
#define STAR_HPP

#include "Coordinates.hpp"
#include <GL/glut.h>

// Define a structure for representing stars in the simulation
struct Star {
  GLfloat brightness;
  Coordinates pos;
};

#endif
