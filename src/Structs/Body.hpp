#ifndef BODY_HPP
#define BODY_HPP

#include "Color.hpp"
#include <GL/glut.h>

// Define a structure for representing celestial bodies
struct Body {
  GLfloat mass;
  GLfloat x, z;
  GLfloat vx, vz;
  GLfloat velocity;
  GLfloat rotatedAngle;
  GLfloat ownAxisRotationVelocity;
  Color color;
  GLfloat simulatedSize;
};

#endif
