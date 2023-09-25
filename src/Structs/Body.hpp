#ifndef BODY_HPP
#define BODY_HPP

#include <GL/glut.h>
#include "Color.hpp"

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
