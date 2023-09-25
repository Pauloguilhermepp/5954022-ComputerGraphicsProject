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
const GLfloat scale = 1 / 1e8;
const int minCamSpeed = 1;
const int maxCamSpeed = 200;
const GLfloat mouseSensitivity = 0.05;
const int gridSize = 1e4;
const int gridSpacing = 1e2;
const int numberOfStars = 1e5;
const int renderDistance = 3e4;

// Define objects representing celestial bodies and camera settings
Body sun, earth, moon, comet;
Star stars[numberOfStars];
GLdouble Px, Py, Pz;
GLfloat fov = 60, fAspect, width = 1200, height = 900, cameraYaw = -90,
        cameraPitch = 0;
Coordinates camera{0, 50, 300}, lookAtHim{camera.x, camera.y, camera.z - 1};
int camSpeed = 10;
int previousMouseX, previousMouseY;
int simulationSpeed = 25;
bool simulationPaused = false;
int Bx[] = {-200, -100, 100, 200};
int By[] = {0, 0, 0, 0};
int Bz[] = {1200, -950, -950, 1200};
double currentCometPosition = 0;
bool showBezierCurve = true;
bool showGrid = true;

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
  glutSolidSphere(body.simulatedSize, 50, 20);
  glPopMatrix();
}

// Function to draw all celestial bodies
void drawBodies() {
  drawBody(sun);
  drawBody(earth);
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
  glBegin(GL_LINE_STRIP);
  for (float t = 0; t <= 1; t = t + 0.02) {
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
  glBegin(GL_LINE_STRIP);
  glVertex3i(Bx[0], By[0], Bz[0]);
  glVertex3i(Bx[1], By[1], Bz[1]);
  glVertex3i(Bx[2], By[2], Bz[2]);
  glVertex3i(Bx[3], By[3], Bz[3]);
  glEnd();
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

  if (showBezierCurve) {
    drawBezierCurve();
    drawBezierRefPoints();
  }

  glutSwapBuffers();
}

// initialize OpenGL settings
void initialize(void) {
  /*
  GLfloat luzAmbiente[4]={0.2, 0.2, 0.2, 1.0};   // {R, G, B, alfa}
  GLfloat luzDifusa[4]={0.5, 0.5, 0.5, 1.0};	   // o 4o componente, alfa,
controla a opacidade/transparência da luz GLfloat
luzEspecular[4]={1.0, 1.0, 1.0, 1.0}; GLfloat
posicaoLuz[4]={50.0, 50.0, 50.0, 1.0};  // aqui o 4o componente indica o tipo de
fonte:
                                              // 0 para luz direcional (no
infinito) e 1 para luz pontual (em x, y, z)

  // Capacidade de brilho do material
  GLfloat especularidade[4]={1.0, 1.0, 1.0, 1.0};
  GLint especMaterial = 100;

  // Especifica a cor de fundo da janela
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Habilita o modelo de colorização de Gouraud
  glShadeModel(GL_SMOOTH);  // a cor de cada ponto da primitiva é interpolada a
partir dos vértices
//glShadeModel(GL_FLAT);  // a cor de cada primitiva é única em todos os pontos

  // Define a refletância do material
  glMaterialfv(GL_FRONT, GL_SPECULAR, especularidade);
  // Define a concentração do brilho
  glMateriali(GL_FRONT, GL_SHININESS, especMaterial);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

  // Define os parâmetros da luz de número 0
  glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa );
  glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular );
  glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz );

  // Habilita a definição da cor do material a partir da cor corrente
  glEnable(GL_COLOR_MATERIAL);
  //Habilita o uso de iluminação
  glEnable(GL_LIGHTING);
  // Habilita a luz de número 0
  glEnable(GL_LIGHT0);
  // Habilita o depth-buffering
  glEnable(GL_DEPTH_TEST);
  */
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
    ax = 0;
    az = 0;

    rotateBody(sun, timeWay);
    updateComet(comet, timeWay);

    calculateGravity(earth, sun, ax, az);
    updateBody(earth, ax, az, timeWay, simulationTimePrecision);

    ax = 0;
    az = 0;

    calculateGravity(moon, earth, ax, az);
    calculateGravity(moon, sun, ax, az);
    updateBody(moon, ax, az, timeWay, simulationTimePrecision);
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
  }
  if (movementKeyPressed.at(' ')) {
    camera.y += camSpeed;
    lookAtHim.y += camSpeed;
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

// Set up properties of celestial bodies
void setBodies() {
  sun.mass = 1.989e30;
  sun.x = 0;
  sun.z = 0;
  sun.vx = 0;
  sun.vz = 0;
  sun.rotatedAngle = 0;
  sun.ownAxisRotationVelocity = 0.004;
  sun.color.r = 1.0;
  sun.color.g = 0.7;
  sun.color.b = 0.0;
  sun.simulatedSize = 50;

  earth.mass = 5.972e24;
  earth.x = 147e9;
  earth.z = 0;
  earth.vx = 0;
  earth.vz = 29783;
  earth.rotatedAngle = 0;
  earth.ownAxisRotationVelocity = 0.1;
  earth.color.r = 0.0;
  earth.color.g = 0.5;
  earth.color.b = 0.3;
  earth.simulatedSize = 3;

  // problemas de escala com a lua
  moon.mass = 7.347e22;
  moon.x = earth.x - 4e8;
  moon.z = earth.z;
  moon.vx = 0;
  moon.vz = earth.vz + 1030;
  moon.rotatedAngle = 0;
  moon.ownAxisRotationVelocity = 0.1;
  moon.color.r = 0.3;
  moon.color.g = 0.3;
  moon.color.b = 0.3;
  moon.simulatedSize = 0.8;

  comet.x = Bx[0] / scale;
  comet.z = Bz[0] / scale;
  comet.velocity = 0.0001;
  comet.color.r = 0.6;
  comet.color.g = 0.6;
  comet.color.b = 0.6;
  comet.rotatedAngle = 0;
  comet.simulatedSize = 10;

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
