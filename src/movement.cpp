#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <map>

#define POSITIVE 1
#define NEGATIVE -1
#define PI 3.14159265358
#define G 6.67430e-11

using namespace std;

// Define a structure to represent celestial bodies
struct Body {
    double mass;
    double x, y;
    double vx, vy;
};

struct Coordinates {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

std::map<char, bool> movementKeyPressed = {
    { 'w', false },
    { 'a', false },
    { 's', false },
	{ 'd', false },
	{ ' ', false },
	{ '\\', false}
};

int count;

const unsigned int simulationTimePrecision = 1; // the lower, the better;
const unsigned int deltaT = 16;
const double scale = 1/5e8;
const int camSpeedChangeRatio = 2;
const int simulationSpeedChangeRatio = 1000;
const int minCamSpeed = 1;
const int maxCamSpeed = 200;
const double mouseSensitivity = 0.1;

Body sun, earth;
GLfloat fov, fAspect, largura, altura, yaw = -90, pitch = 0;
Coordinates camera{ 0, 0, 0 }, lookAtHim{ camera.x, camera.y, camera.z-1 };
int camSpeed = 10;
int previousMouseX, previousMouseY;
int simulationSpeed;

void LogCoordinates(Coordinates coordinates) {
	cout << "x: " << coordinates.x << endl;
	cout << "y: " << coordinates.y << endl;
	cout << "z: " << coordinates.z << endl;
}

// Function to calculate gravitational force between two bodies
void CalculateGravity(Body& body1, Body& body2, double& fx, double& fy) {
    double dx = body2.x - body1.x;
    double dy = body2.y - body1.y;
    double r = sqrt(dx * dx + dy * dy);

    double F = (G * body1.mass * body2.mass) / (r * r);

    fx = F * (dx / r);
    fy = F * (dy / r);
}

// Function to update the position and velocity of a body based on forces
void UpdateBody(Body& body, double fx, double fy, int dt) {
    double ax = fx / body.mass;
    double ay = fy / body.mass;

    body.vx += ax * dt;
    body.vy += ay * dt;

    body.x += body.vx * dt;
    body.y += body.vy * dt;
}

void DrawCrosshair() {
	glColor3f(0, 0, 0);
	glPushMatrix();
        glTranslated(lookAtHim.x, lookAtHim.y, lookAtHim.z);
		glutWireSphere(0.001, 10, 10);
    glPopMatrix();
}

void DrawBodies() {
	glColor3f(8.0f, 0.5f, 0.5f);
    glPushMatrix();
        glTranslated(0, 0, 0);
        glutSolidSphere(50, 20, 20);
    glPopMatrix();

	glColor3f(0, 0.5f, 0.2f);
    glPushMatrix();
        glTranslated(earth.x*scale, earth.y*scale, 0);
        glutSolidSphere(5, 20, 20);
    glPopMatrix();
}

void Desenha(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, largura, altura);

	DrawCrosshair();
	DrawBodies();

	glutSwapBuffers();
}

void Inicializa(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // glEnable(GL_DEPTH_TEST);   //ativa o zBuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   //aplica o zBuffer

	fov = 45;
}

void EspecificaParametrosVisualizacao(void)
{
	// Especifica sistema de coordenadas de projeção
	glMatrixMode(GL_PROJECTION);
	// Inicializa sistema de coordenadas de projeção
	glLoadIdentity();

	gluPerspective(fov, fAspect, 0.1, 10000);

	// Especifica sistema de coordenadas do modelo
	glMatrixMode(GL_MODELVIEW);
	// Inicializa sistema de coordenadas do modelo
	glLoadIdentity();

	gluLookAt(camera.x, camera.y, camera.z,
              lookAtHim.x, lookAtHim.y, lookAtHim.z,
              0, 1, 0);
}

// Função callback chamada quando o tamanho da janela é alterado 
void AlteraTamanhoJanela(GLint largura, GLint altura)
{
	// Para previnir uma divisão por zero
	if (altura == 0) altura = 1;

	glViewport(0, 0, largura, altura);

	// Calcula a correção de aspecto
	fAspect = (GLfloat)largura / (GLfloat)altura;

	EspecificaParametrosVisualizacao();
}

Coordinates lookAtFloorDirection() {
	GLfloat deltaX = lookAtHim.x - camera.x, deltaZ = lookAtHim.z - camera.z;
	GLfloat size = sqrt(deltaX*deltaX + deltaZ*deltaZ);
	if (size == 0)
	{
		return { 0, 0, 1 };
	}
	
	return { deltaX/size, 0, deltaZ/size };
}

void ResetMouse() {
	previousMouseX = largura/2;
	previousMouseY = altura/2;
	glutWarpPointer(largura/2, altura/2);
}

void UpdateCamera(bool sidesOrientation, int way) {
	GLfloat deltaX, deltaZ;
	Coordinates direction = lookAtFloorDirection();

	if (sidesOrientation) {
		deltaX = -way*camSpeed*direction.z;
		deltaZ = way*camSpeed*direction.x;
	} else {
		deltaX = way*camSpeed*direction.x;
		deltaZ = way*camSpeed*direction.z;
	}
	
	camera.x += deltaX;
	camera.z += deltaZ;
	lookAtHim.x += deltaX;
	lookAtHim.z += deltaZ;
}

GLfloat radians(GLfloat degree) {
	return degree * (PI/180);
}

void GerenciaMovimentoMouse(int x, int y) {
	if (x > largura-100 || y > altura-100 || x < 100 || y < 100) {
		ResetMouse();
		return;
	}

	int deltaX = x - previousMouseX;
	int deltaY = y - previousMouseY;

	previousMouseX = x;
	previousMouseY = y;

	yaw += deltaX*mouseSensitivity;	
	pitch -= deltaY*mouseSensitivity;
	if (pitch >= 90) pitch = 89.9; else if (pitch <= -90) pitch = -89.9;

	lookAtHim.x = camera.x + cos(radians(yaw)) * cos(radians(pitch));
	lookAtHim.y = camera.y + sin(radians(pitch));
	lookAtHim.z = camera.z + sin(radians(yaw)) * cos(radians(pitch));
	
	EspecificaParametrosVisualizacao();
	glutPostRedisplay();
}

void MouseClick(int button, int state, int x, int y)
{
    switch (button)
    {
    case 3:
        camSpeed = camSpeed > maxCamSpeed ? maxCamSpeed : camSpeed + camSpeedChangeRatio;
        break;
    case 4:
        camSpeed = camSpeed < minCamSpeed ? minCamSpeed : camSpeed - camSpeedChangeRatio;
        break;
    case GLUT_LEFT_BUTTON:
        break;
    }
}

void SpecialKeys(int key, int x, int y)
{
    switch (key)
    {
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

	// EspecificaParametrosVisualizacao();
	// glutPostRedisplay();
}

void KeyboardFunc(unsigned char key, int x, int y) {
	// setar o map
    switch (key)
    {
    case 'w':
        UpdateCamera(false, POSITIVE);
        break;
    case 's':
		UpdateCamera(false, NEGATIVE);
        break;
    case 'd':
        UpdateCamera(true, POSITIVE);
        break;
    case 'a':
        UpdateCamera(true, NEGATIVE);
        break;
	case '\\':
		camera.y -= camSpeed;
		lookAtHim.y -= camSpeed;
		break;
	case ' ':
		camera.y += camSpeed;
		lookAtHim.y += camSpeed;
		break;
    }

	EspecificaParametrosVisualizacao();
	glutPostRedisplay();
}

void KeyboardUpFunc(unsigned char key, int x, int y) {
	cout << count++ << endl;
}

void SimulationTick() {
	int absSimulationSpeed = abs(simulationSpeed);
	// timeWay: -1 (backwards in time) or 1 (forwards in time)
	int timeWay = absSimulationSpeed/simulationSpeed;

	for (int t = 0; t < absSimulationSpeed; t++) {
        double fx, fy;

        CalculateGravity(earth, sun, fx, fy);
        UpdateBody(earth, fx, fy, timeWay*simulationTimePrecision);
    }
}

void Timer(int _ = 0) {
	// verificar o map
	SimulationTick();

    glutPostRedisplay();
    glutTimerFunc(deltaT, Timer, _);
}

void SetBodies() {
    // ifstream inputFile("Data/data.txt");
	// double _;

    // inputFile >> simulationSpeed >> _;
    // inputFile >> sun.mass >> sun.x >> sun.y >> sun.vx >> sun.vy;
    // inputFile >> earth.mass >> earth.x >> earth.y >> earth.vx >> earth.vy;
	simulationSpeed = 3600;

	sun.mass = 1.989e30;
	sun.x = 0;
	sun.y = 0;
	sun.vx = 0;
	sun.vy = 0;

	earth.mass = 5.972e24;
	earth.x = 147e9;
	earth.y = 0;
	earth.vx = 0;
	earth.vy = 29783;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(0,0);
    largura = 1500;
    altura = 1000;
	fAspect = (GLfloat)largura / (GLfloat)altura;
    glutInitWindowSize(largura,altura);
    glutCreateWindow("Aula Pratica 4");
	glutSetCursor(GLUT_CURSOR_NONE);
	ResetMouse();
	SetBodies();
	Timer();
	glutDisplayFunc(Desenha);
	glutReshapeFunc(AlteraTamanhoJanela); // Função para ajustar o tamanho da tela
    glutMouseFunc(MouseClick);
	// glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(KeyboardFunc); // Define qual funcao gerencia o comportamento do teclado
	glutKeyboardUpFunc(KeyboardUpFunc);
    glutSpecialFunc(SpecialKeys); // Define qual funcao gerencia as teclas especiais
	glutPassiveMotionFunc(GerenciaMovimentoMouse);
	Inicializa();
	glutMainLoop();
}
