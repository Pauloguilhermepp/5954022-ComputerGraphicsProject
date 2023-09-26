#include <GL/glut.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "Structs/Body.hpp"
#include "Structs/Coordinates.hpp"
#include "Structs/Star.hpp"

#define POSITIVE 1
#define NEGATIVE -1
#define PI 3.14159265358
#define G 6.67430e-11

using namespace std;

// Create a map to track movement key presses
map<char, bool> movementKeyPressed = {{'w', false}, {'a', false},
                                      {'s', false}, {'d', false},
                                      {' ', false}, {'\\', false}};

// Constants and parameters used in the simulation
const unsigned int simulationTimePrecision = 1000; // the lower, the better;
const int simulationSpeedChangeRatio = 10;
const unsigned int deltaT = 16;
const GLfloat scale = 1 / 5e7;
const int minCamSpeed = 1;
const int maxCamSpeed = 200;
const GLfloat mouseSensitivity = 0.05;
const int gridSize = 1e5;
const int numberOfStars = 1e5;
const int renderDistance = 1e5;

// Define objects representing celestial bodies and camera settings
Body sun, moon, comet;
map<string, Body> planets;
Star stars[numberOfStars];
GLdouble Px, Py, Pz;
GLfloat fov = 60, fAspect, width = 1200, height = 900, cameraYaw = -90,
        cameraPitch = 0;
Coordinates camera{0, 500, 300}, lookAtHim{camera.x, camera.y, camera.z - 1};
int camSpeed = 10;
int previousMouseX, previousMouseY;
int simulationSpeed = 25;
bool simulationPaused = false;
int Bx[] = {-5000, -1000, 1000, 5000};
int By[] = {0, 0, 0, 0};
int Bz[] = {20000, -8000, -8000, 20000};
bool findPlanet[] = {false, false, false, false, false, false, false, false};
string planetsNames[] = {"Mercury", "Venus",  "Earth",  "Mars",
                         "Jupiter", "Saturn", "Uranus", "Neptune"};
double currentCometPosition = 0;
bool showBezierCurve = true;
bool showGrid = true;
int gridSpacing = 2e2;
int nextYLimitDelta = 1000;
int nextYLimit = nextYLimitDelta;

// Function to log coordinates for debugging purposes
void logCoordinates(Coordinates coordinates) {
  cout << "x: " << coordinates.x << endl;
  cout << "y: " << coordinates.y << endl;
  cout << "z: " << coordinates.z << endl;
}

// Function to calculate Bezier point coordinates
float calculateBezierPoint(char c, double t) {
  switch (c) {
  case 'x':
    return (pow(1 - t, 3) * Bx[0] + 3 * t * pow(1 - t, 2) * Bx[1] +
            3 * pow(t, 2) * (1 - t) * Bx[2] + pow(t, 3) * Bx[3]);
  case 'y':
    return (pow(1 - t, 3) * By[0] + 3 * t * pow(1 - t, 2) * By[1] +
            3 * pow(t, 2) * (1 - t) * By[2] + pow(t, 3) * By[3]);
  case 'z':
    return (pow(1 - t, 3) * Bz[0] + 3 * t * pow(1 - t, 2) * Bz[1] +
            3 * pow(t, 2) * (1 - t) * Bz[2] + pow(t, 3) * Bz[3]);
  default:
    // Print an error message and exit the program
    std::cerr << "Invalid character '" << c
              << "' passed to calculateBezierPoint." << std::endl;
    exit(EXIT_FAILURE);
  }
}

// Function to calculate gravitational force between two bodies
void calculateGravity(Body &body1, Body &body2, GLfloat &ax, GLfloat &az) {
  GLfloat dx = body2.x - body1.x;
  GLfloat dz = body2.z - body1.z;
  GLfloat r = sqrt(dx * dx + dz * dz);

  GLfloat F = (G * body1.mass * body2.mass) / (r * r);

  ax += (F * (dx / r)) / body1.mass;
  az += (F * (dz / r)) / body1.mass;
}

// Function to update the rotation of a body
void rotateBody(Body &body, int timeWay) {
  body.rotatedAngle += timeWay * body.ownAxisRotationVelocity;
}

// Function to update the position, velocity, and rotation of a body
void updateBody(Body &body, GLfloat ax, GLfloat az, int timeWay, int dt) {
  dt *= timeWay;

  body.vx += ax * dt;
  body.vz += az * dt;

  body.x += body.vx * dt;
  body.z += body.vz * dt;

  rotateBody(body, timeWay);
}

// Function to update comet's position
void updateComet(Body &body, int timeWay) {
  currentCometPosition += timeWay * body.velocity;

  if (currentCometPosition > 1) {
    currentCometPosition = 0;
  } else if (currentCometPosition < 0) {
    currentCometPosition = 1;
  }

  body.x = calculateBezierPoint('x', currentCometPosition) / scale;
  body.z = calculateBezierPoint('z', currentCometPosition) / scale;
}

// Function to draw a crosshair at the center of the screen
void drawCrosshair() {
  glColor3f(0, 1, 1);
  glBegin(GL_POINTS);
  glVertex3f(lookAtHim.x, lookAtHim.y, lookAtHim.z);
  glEnd();
}

// Function to draw a celestial body
void drawBody(Body body) {
  glColor3f(body.color.r, body.color.g, body.color.b);
  glPushMatrix();
  glTranslated(body.x * scale, 0, body.z * scale);
  glRotatef(body.rotatedAngle, 0.0, 1.0, 0.0);
  glutSolidSphere(body.simulatedSize, 80, 20);
  glPopMatrix();
}

// Function to draw a ring
void drawRing(Body planet, GLfloat innerRadius, GLfloat outerRadius,
              int sides) {
  glBegin(GL_TRIANGLE_STRIP);
  glLineWidth(4.0f);
  glColor3f(planet.color.r, planet.color.g, planet.color.b);
  for (int i = 0; i <= sides; ++i) {
    GLfloat angle = 2.0f * M_PI * i / sides;
    GLfloat x = cos(angle);
    GLfloat z = sin(angle);

    // Outer vertex
    glVertex3f((planet.x * scale + outerRadius * x), 0,
               (planet.z * scale + outerRadius * z));

    // Inner vertex
    glVertex3f((planet.x * scale + innerRadius * x), 0,
               (planet.z * scale + innerRadius * z));
  }
  glEnd();
}

// Function do draw all the planets
void drawPlanets() {
  for (auto &x : planets) {
    drawBody(x.second);

    if (x.first == "Saturn") {
      drawRing(x.second, 1.2 * x.second.simulatedSize,
               1.5 * x.second.simulatedSize, 50);
    }
  }
}

// Function to draw all celestial bodies
void drawBodies() {
  drawBody(sun);
  drawPlanets();
  drawBody(moon);
  drawBody(comet);
}

// Function to draw stars
void drawStars() {
  glBegin(GL_POINTS);
  for (int i = 0; i < numberOfStars; i++) {
    glColor3f(stars[i].brightness, stars[i].brightness, stars[i].brightness);
    glVertex3f(stars[i].pos.x + camera.x, stars[i].pos.y + camera.y,
               stars[i].pos.z + camera.z);
  }
  glEnd();
}

// Function to draw a grid in the X-Z plane
void drawXZPlaneGrid() {
  glColor3f(0.3, 0.3, 0.3);
  glLineWidth(1.0f);
  glBegin(GL_LINES);
  for (int i = -gridSize; i <= gridSize; i += gridSpacing) {
    glVertex3f(i, 0, -gridSize);
    glVertex3f(i, 0, gridSize);
    glVertex3f(-gridSize, 0, i);
    glVertex3f(gridSize, 0, i);
  };
  glEnd();
}

// Function to draw the Bezier curve
void drawBezierCurve() {
  glColor3f(1.0f, 0.5f, 0.0f);
  glLineWidth(4.0f);
  glBegin(GL_LINE_STRIP);
  for (float t = 0; t <= 1; t = t + 0.01) {
    Px = calculateBezierPoint('x', t);
    Py = calculateBezierPoint('y', t);
    Pz = calculateBezierPoint('z', t);
    glVertex3d(Px, Py, Pz);
  }
  glEnd();
}

// Function to draw the reference points of the Bezier curve
void drawBezierRefPoints() {
  glColor3f(0.5f, 0.0f, 0.5f);
  glLineWidth(4.0f);
  glBegin(GL_LINE_STRIP);
  glVertex3i(Bx[0], By[0], Bz[0]);
  glVertex3i(Bx[1], By[1], Bz[1]);
  glVertex3i(Bx[2], By[2], Bz[2]);
  glVertex3i(Bx[3], By[3], Bz[3]);
  glEnd();
}

void drawFindPlanet() {
  // Initialize an iterator to the beginning of the map
  Body planet;
  for (int i = 0; i < 8; i++) {
    if (findPlanet[i]) {
      planet = planets.at(planetsNames[i]);
      glColor3f(planet.color.r, planet.color.g, planet.color.b);
      glLineWidth(2.0f);
      glBegin(GL_LINE_STRIP);
      glVertex3i(camera.x, camera.y - 10, camera.z);
      glVertex3i(planet.x * scale, 0, planet.z * scale);
      glEnd();
    }
  }
}

// Function to render the entire scene, including stars, celestial bodies, and
// grid
void renderScene(void) {
  glViewport(0, 0, width, height);

  // Draw the stars before all other things and clearing the depth buffer, so
  // the stars are behind everything
  glClear(GL_COLOR_BUFFER_BIT);
  drawStars();
  glClear(GL_DEPTH_BUFFER_BIT);

  drawCrosshair();

  if (showGrid) {
    drawXZPlaneGrid();
  }
  drawBodies();

  drawFindPlanet();

  if (showBezierCurve) {
    drawBezierCurve();
    drawBezierRefPoints();
  }

  glutSwapBuffers();
}

// initialize OpenGL settings
void initialize(void) {
  GLfloat luzDifusa[4] = {1, 1, 1, 1};
  GLfloat posicaoLuz[4] = {sun.x, 0, sun.z, 1.0};

  // Capacidade de brilho do material
  GLfloat especularidade[4] = {1.0, 1.0, 1.0, 1.0};
  GLfloat zero[4] = {0,0,0,1};
  GLint especMaterial = 127;

  // Habilita o modelo de colorização de Gouraud
  glShadeModel(GL_SMOOTH);
  // glShadeModel(GL_FLAT);

  // Define a refletância do material
  glMaterialfv(GL_FRONT, GL_DIFFUSE, luzDifusa);
  glMaterialfv(GL_BACK, GL_DIFFUSE, zero);
  glMaterialfv(GL_FRONT | GL_BACK, GL_AMBIENT, zero);
  glMaterialfv(GL_FRONT | GL_BACK, GL_SPECULAR, zero);
  // Define a concentração do brilho
  glMateriali(GL_FRONT, GL_SHININESS, especMaterial);
  glMateriali(GL_BACK, GL_SHININESS, 0);

  // Define os parâmetros da luz de número 0
  glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
  glLightfv(GL_LIGHT0, GL_AMBIENT, zero);
  glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
  glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz );

  // Habilita a definição da cor do material a partir da cor corrente
  glEnable(GL_COLOR_MATERIAL);
  //Habilita o uso de iluminação
  glEnable(GL_LIGHTING);
  // Habilita a luz de número 0
  glEnable(GL_LIGHT0);
  
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  glEnable(GL_DEPTH_TEST);                            // Turning on zBuffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Applying zBuffer
}

// Set up the viewing parameters
void specifyViewingParameters(void) {
  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();

  gluPerspective(fov, fAspect, 0.1, renderDistance);

  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();

  gluLookAt(camera.x, camera.y, camera.z, lookAtHim.x, lookAtHim.y, lookAtHim.z,
            0, 1, 0);
}

// Callback function for window resizing
void resizeWindow(GLint width, GLint height) {
  // To prevent division by zero
  if (height == 0)
    height = 1;

  glViewport(0, 0, width, height);

  // Calculate aspect ratio correction
  fAspect = (GLfloat)width / (GLfloat)height;

  specifyViewingParameters();
}

// Calculate the direction vector pointing to the floor
Coordinates lookAtFloorDirection() {
  GLfloat deltaX = lookAtHim.x - camera.x, deltaZ = lookAtHim.z - camera.z;
  GLfloat size = sqrt(deltaX * deltaX + deltaZ * deltaZ);
  if (size == 0) {
    return {0, 0, 1};
  }

  return {deltaX / size, 0, deltaZ / size};
}

// Reset the mouse to the center of the window
void resetMouse() {
  previousMouseX = width / 2;
  previousMouseY = height / 2;
  glutWarpPointer(width / 2, height / 2);
}

// Convert degrees to radians
GLfloat degreesToRadians(GLfloat degree) { return degree * (PI / 180); }

// Update the camera's position based on user input
void updateCamera(bool sidesOrientation, int way) {
  GLfloat deltaX, deltaZ;
  Coordinates direction = lookAtFloorDirection();

  if (sidesOrientation) {
    deltaX = -way * camSpeed * direction.z;
    deltaZ = way * camSpeed * direction.x;
  } else {
    deltaX = way * camSpeed * direction.x;
    deltaZ = way * camSpeed * direction.z;
  }

  camera.x += deltaX;
  camera.z += deltaZ;
  lookAtHim.x += deltaX;
  lookAtHim.z += deltaZ;
}

// Handle mouse movement to control the camera
void handleMouseMovement(int x, int y) {
  if (x > width - 200 || y > height - 200 || x < 200 || y < 200) {
    resetMouse();
    return;
  }

  int deltaX = x - previousMouseX;
  int deltaY = y - previousMouseY;

  previousMouseX = x;
  previousMouseY = y;

  cameraYaw += deltaX * mouseSensitivity;
  cameraPitch -= deltaY * mouseSensitivity;
  if (cameraPitch >= 90)
    cameraPitch = 89.9;
  else if (cameraPitch <= -90)
    cameraPitch = -89.9;

  lookAtHim.x = camera.x + cos(degreesToRadians(cameraYaw)) *
                               cos(degreesToRadians(cameraPitch));
  lookAtHim.y = camera.y + sin(degreesToRadians(cameraPitch));
  lookAtHim.z = camera.z + sin(degreesToRadians(cameraYaw)) *
                               cos(degreesToRadians(cameraPitch));

  specifyViewingParameters();
  glutPostRedisplay();
}

// Handle mouse clicks
void handleMouseClick(int button, int state, int x, int y) {
  const int camSpeedChangeRatio = 2;
  switch (button) {
  case 3:
    camSpeed =
        camSpeed > maxCamSpeed ? maxCamSpeed : camSpeed + camSpeedChangeRatio;
    break;
  case 4:
    camSpeed =
        camSpeed < minCamSpeed ? minCamSpeed : camSpeed - camSpeedChangeRatio;
    break;
  case GLUT_LEFT_BUTTON:
    break;
  }
}

// Handle special keyboard keys
void handleSpecialKeys(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_LEFT:
    break;
  case GLUT_KEY_RIGHT:
    break;
  case GLUT_KEY_UP:
    simulationSpeed += simulationSpeedChangeRatio;
    break;
  case GLUT_KEY_DOWN:
    simulationSpeed -= simulationSpeedChangeRatio;
    break;
  }
}

// Check if a key is a movement key
bool isMovementKey(unsigned char key) {
  return movementKeyPressed.find(key) != movementKeyPressed.end();
}

// Check if character is between 1 and 8
bool isValidNumber(char c) { return c >= '1' && c <= '8'; }

// Check if the path to a planet should be displayed
void checkFindPlanet(char c) {
  int num;
  if (isValidNumber(c)) {
    num = (c - '0') - 1;
    findPlanet[num] = !findPlanet[num];
  }
}

// Handle regular keyboard key presses
void handleKeyboard(unsigned char key, int x, int y) {
  const int fovChangeRatio = 1;

  if (isMovementKey(key)) {
    movementKeyPressed.at(key) = true;
  }

  switch (key) {
  case 13: // Enter
    simulationPaused = !simulationPaused;
    break;
  case '=':
    fov -= fovChangeRatio;
    break;
  case '-':
    fov += fovChangeRatio;
    break;
  case 'b':
    showBezierCurve = !showBezierCurve;
    break;
  case 'g':
    showGrid = !showGrid;
    break;
  default:
    checkFindPlanet(key);
    break;
  }
}

// Handle keyboard key releases
void handleKeyboardUp(unsigned char key, int x, int y) {
  if (isMovementKey(key)) {
    movementKeyPressed.at(key) = false;
  }
}

// Update the simulation state for a time step
void simulationTick() {
  GLfloat ax, az;
  int absSimulationSpeed = abs(simulationSpeed);
  // timeWay: -1 (backwards in time) or 1 (forwards in time)
  int timeWay = absSimulationSpeed / simulationSpeed;

  for (int t = 0; t < absSimulationSpeed; t++) {
    rotateBody(sun, timeWay);
    updateComet(comet, timeWay);

    for (auto &x : planets) {
      ax = 0;
      az = 0;
      calculateGravity(x.second, sun, ax, az);
      updateBody(x.second, ax, az, timeWay, simulationTimePrecision);
    }

    ax = 0;
    az = 0;

    calculateGravity(moon, planets.at("Earth"), ax, az);
    calculateGravity(moon, sun, ax, az);
    updateBody(moon, ax, az, timeWay, simulationTimePrecision);
  }
}

void shrinkGrid() {
  gridSpacing /= 2;
  nextYLimitDelta /= 2;
  nextYLimit -= nextYLimitDelta;
}

void expandGrid() {
  nextYLimit += nextYLimitDelta;
  nextYLimitDelta *= 2;
  gridSpacing *= 2;
}

void updateGrid() {
  int minDeltaYToUpdate = 500;
  int deltaY = abs(camera.y);
  if (deltaY < nextYLimit-nextYLimitDelta/2 && deltaY > minDeltaYToUpdate) {
    shrinkGrid();
  } else if (deltaY > nextYLimit && deltaY > minDeltaYToUpdate) {
    expandGrid();
  }
}

// Update camera movement based on user input
void updateMovement() {
  if (movementKeyPressed.at('w')) {
    updateCamera(false, POSITIVE);
  }
  if (movementKeyPressed.at('a')) {
    updateCamera(true, NEGATIVE);
  }
  if (movementKeyPressed.at('s')) {
    updateCamera(false, NEGATIVE);
  }
  if (movementKeyPressed.at('d')) {
    updateCamera(true, POSITIVE);
  }
  if (movementKeyPressed.at('\\')) {
    camera.y -= camSpeed;
    lookAtHim.y -= camSpeed;
    updateGrid();
  }
  if (movementKeyPressed.at(' ')) {
    camera.y += camSpeed;
    lookAtHim.y += camSpeed;
    updateGrid();
  }
}

// Timer function to update the simulation and rendering
void timer(int _ = 0) {
  updateMovement();
  if (simulationSpeed != 0 && !simulationPaused) {
    simulationTick();
  }

  specifyViewingParameters();
  glutPostRedisplay();
  glutTimerFunc(deltaT, timer, _);
}

// Initialize stars and celestial bodies
void initStars() {
  int yaw, pitch;
  GLfloat brightness;
  for (int i = 0; i < numberOfStars; i++) {
    yaw = rand();
    pitch = rand();
    brightness = rand() / GLfloat(RAND_MAX);
    stars[i].pos.x = cos(degreesToRadians(yaw)) * cos(degreesToRadians(pitch));
    stars[i].pos.y = sin(degreesToRadians(pitch));
    stars[i].pos.z = sin(degreesToRadians(yaw)) * cos(degreesToRadians(pitch));
    stars[i].brightness = brightness;
  }
}

// Function to set a new color
Color setColor(GLfloat cr, GLfloat cg, GLfloat cb) {
  Color color;
  color.r = cr;
  color.g = cg;
  color.b = cb;

  return color;
}

// Function to create a complete celestial body
Body setBody(GLfloat mass, GLfloat x, GLfloat z, GLfloat vx, GLfloat vz,
             GLfloat velocity, GLfloat rotatedAngle,
             GLfloat ownAxisRotationVelocity, Color color,
             GLfloat simulatedSize) {
  Body body;
  body.mass = mass;
  body.x = x;
  body.z = z;
  body.vx = vx;
  body.vz = vz;
  body.velocity = velocity;
  body.rotatedAngle = rotatedAngle;
  body.ownAxisRotationVelocity = ownAxisRotationVelocity;
  body.color.r = color.r;
  body.color.g = color.g;
  body.color.b = color.b;
  body.simulatedSize = simulatedSize;

  return body;
}

// Set up properties of all celestial bodies
void setBodies() {
  // mass, x, z, vx, vz, velocity, rotatedAngle, ownAxisRotationVelocity, color,
  // simulatedSize
  sun = setBody(1.989e30, 0, 0, 0, 0, 0, 0, 0.004, setColor(1, 0.7, 0.0), 500);

  planets.insert({"Mercury", setBody(3.3011e23, 57.9e9, 0, 0, 47.87e3, 0, 0,
                                     0.1, setColor(0.8, 0.8, 0.8), 8)});
  planets.insert({"Venus", setBody(4.8675e24, 108.2e9, 0, 0, 35.02e3, 0, 0, 0.1,
                                   setColor(0.9, 0.8, 0.6), 10)});
  planets.insert({"Earth", setBody(5.972e24, 147.1e9, 0, 0, 29.78e3, 0, 0, 0.1,
                                   setColor(0.0, 0.5, 0.3), 10)});
  planets.insert({"Mars", setBody(6.4171e23, 227.9e9, 0, 0, 24.077e3, 0, 0, 0.1,
                                  setColor(0.9, 0.2, 0.1), 9)});
  planets.insert({"Jupiter", setBody(1.8982e27, 778.3e9, 0, 0, 13.07e3, 0, 0,
                                     0.1, setColor(0.9, 0.6, 0.4), 200)});
  planets.insert({"Saturn", setBody(5.6834e26, 1.42e12, 0, 0, 9.69e3, 0, 0, 0.1,
                                    setColor(0.8, 0.7, 0.5), 150)});
  planets.insert({"Uranus", setBody(8.6810e25, 2.87e12, 0, 0, 6.81e3, 0, 0, 0.1,
                                    setColor(0.6, 0.8, 0.8), 100)});
  planets.insert({"Neptune", setBody(1.02413e26, 4.5e12, 0, 0, 5.43e3, 0, 0,
                                     0.1, setColor(0.1, 0.1, 0.9), 100)});

  moon = setBody(7.347e22, 147e9 - 4e8, 0, 0, 29783 + 1030, 0, 0, 0.1,
                 setColor(0.3, 0.3, 0.3), 1.5);
  comet = setBody(0, Bx[0] / scale, Bz[0] / scale, 0, 0, 0.0001, 0, 0,
                  setColor(0.6, 0.6, 0.6), 20);

  initStars();
}

// Main function of the simulation
int main(int argc, char **argv) {
  setBodies();
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowPosition(0, 0);
  fAspect = width / height;
  glutInitWindowSize(width, height);
  glutCreateWindow("Solar System Simulation");
  glutSetCursor(GLUT_CURSOR_NONE);
  resetMouse();
  timer();
  glutDisplayFunc(renderScene);
  glutReshapeFunc(resizeWindow);
  glutMouseFunc(handleMouseClick);
  glutKeyboardFunc(handleKeyboard);
  glutKeyboardUpFunc(handleKeyboardUp);
  glutSpecialFunc(handleSpecialKeys);
  glutPassiveMotionFunc(handleMouseMovement);
  initialize();
  glutMainLoop();
  return 0;
}
