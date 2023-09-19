#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>

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

const int camAngleChangeRatio = 1;
const int scrollChangeRatio = 2;
const int minCamSpeed = 1;
const int maxCamSpeed = 200;
const int deltaT = 100;
const double mouseSensitivity = 0.1;

Body sun, earth;
GLfloat fov, fAspect, largura, altura, yaw = -90, pitch = 0;
Coordinates camera{ 0, 0, 0 }, lookAtHim{ camera.x, camera.y, camera.z-1 };
int camSpeed = 10;
int previousMouseX, previousMouseY;
int simulationTime;

void logCoordinates(Coordinates coordinates) {
	cout << "x: " << coordinates.x << endl;
	cout << "y: " << coordinates.y << endl;
	cout << "z: " << coordinates.z << endl;
}

// Function to calculate gravitational force between two bodies
void calculateGravity(Body& body1, Body& body2, double& fx, double& fy) {
    double dx = body2.x - body1.x;
    double dy = body2.y - body1.y;
    double r = sqrt(dx * dx + dy * dy);

    double F = (G * body1.mass * body2.mass) / (r * r);

    fx = F * (dx / r);
    fy = F * (dy / r);
}

// Function to update the position and velocity of a body based on forces
void updateBody(Body& body, double fx, double fy, double dt) {
    double ax = fx / body.mass;
    double ay = fy / body.mass;

    body.vx += ax * dt;
    body.vy += ay * dt;

    body.x += body.vx * dt;
    body.y += body.vy * dt;
}

void drawCrosshair() {
	glColor3f(0, 0, 0);
	glPushMatrix();
        glTranslated(lookAtHim.x, lookAtHim.y, lookAtHim.z);
		glutWireSphere(0.001, 10, 10);
    glPopMatrix();
}

void drawBodies() {
	glColor3f(8.0f, 0.5f, 0.5f);
    glPushMatrix();
        glTranslated(0, 0, 0);
        glutSolidSphere(50, 20, 20);
    glPopMatrix();

	glColor3f(0, 0.5f, 0.2f);
    glPushMatrix();
        glTranslated(100, 0, 0);
        glutSolidSphere(5, 20, 20);
    glPopMatrix();

	// cout << "earth x: " << earth.x << endl;
	// cout << "earth y: " << earth.y << endl;
}

void Desenha(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, largura, altura);

	drawCrosshair();
	drawBodies();

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

void resetMouse() {
	previousMouseX = largura/2;
	previousMouseY = altura/2;
	glutWarpPointer(largura/2, altura/2);
}

void updateCamera(bool sidesOrientation, int way) {
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

void updateLookAt(bool moveYaw, int way) {
	if (moveYaw) {
		yaw += way*camAngleChangeRatio;
	} else {
		pitch += way*camAngleChangeRatio;
	}
	
	lookAtHim.x = camera.x + cos(radians(yaw)) * cos(radians(pitch));
	lookAtHim.y = camera.y + sin(radians(pitch));
	lookAtHim.z = camera.z + sin(radians(yaw)) * cos(radians(pitch));
}

void GerenciaMovimentoMouse(int x, int y) {
	if (x > largura-100 || y > altura-100 || x < 100 || y < 100) {
		resetMouse();
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

// Função callback chamada para gerenciar eventos do mouse
void GerenciaCliqueMouse(int button, int state, int x, int y)
{
    switch (button)
    {
    case 3:
        camSpeed = camSpeed > maxCamSpeed ? maxCamSpeed : camSpeed + scrollChangeRatio;
        break;
    case 4:
        camSpeed = camSpeed < minCamSpeed ? minCamSpeed : camSpeed - scrollChangeRatio;
        break;
    case GLUT_LEFT_BUTTON:
        // if (state == GLUT_DOWN)
        break;
    }        

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   //aplica o zBuffer  
    EspecificaParametrosVisualizacao();
	glutPostRedisplay();
}

void TeclasEspeciais(int key, int x, int y)
{
    switch (key)
    {
		case GLUT_KEY_LEFT:
			updateLookAt(true, NEGATIVE);
			break;
		case GLUT_KEY_RIGHT:
			updateLookAt(true, POSITIVE);
			break;
		case GLUT_KEY_UP:
			updateLookAt(false, POSITIVE);
			break;
		case GLUT_KEY_DOWN:
			updateLookAt(false, NEGATIVE);
			break;
    }

	EspecificaParametrosVisualizacao();
	glutPostRedisplay();
}

void GerenciaTeclado(unsigned char key, int x, int y) {
    switch (key)
    {
    case 'w':
        updateCamera(false, POSITIVE);
        break;
    case 's':
		updateCamera(false, NEGATIVE);
        break;
    case 'd':
        updateCamera(true, POSITIVE);
        break;
    case 'a':
        updateCamera(true, NEGATIVE);
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

void Timer(int _ = 0)
{ 
    double fx, fy;
    calculateGravity(earth, sun, fx, fy);
    updateBody(earth, fx, fy, simulationTime);

    glutPostRedisplay();
    glutTimerFunc(deltaT, Timer, _);
}

void setBodies() {
    ifstream inputFile("Data/data.txt");
	double _;

    inputFile >> simulationTime >> _;
    inputFile >> sun.mass >> sun.x >> sun.y >> sun.vx >> sun.vy;
    inputFile >> earth.mass >> earth.x >> earth.y >> earth.vx >> earth.vy;
	cout << "earth x: " << earth.x << endl;
	cout << "earth y: " << earth.y << endl << endl;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  //GLUT_DOUBLE trabalha com dois buffers: um para renderização e outro para exibição
    glutInitWindowPosition(0,0);
    largura = 1500;
    altura = 1000;
	fAspect = (GLfloat)largura / (GLfloat)altura;
    glutInitWindowSize(largura,altura);
    glutCreateWindow("Aula Pratica 4");
	glutSetCursor(GLUT_CURSOR_NONE);
	resetMouse();
	setBodies();
	Timer();
	glutDisplayFunc(Desenha);
	glutReshapeFunc(AlteraTamanhoJanela); // Função para ajustar o tamanho da tela
    glutMouseFunc(GerenciaCliqueMouse);
    glutKeyboardFunc(GerenciaTeclado); // Define qual funcao gerencia o comportamento do teclado
    glutSpecialFunc(TeclasEspeciais); // Define qual funcao gerencia as teclas especiais
	glutPassiveMotionFunc(GerenciaMovimentoMouse);
	Inicializa();
	glutMainLoop();
}
