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
    double x, z;
    double vx, vz;
};

struct Coordinates {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

map<char, bool> movementKeyPressed = {
    { 'w', false },
    { 'a', false },
    { 's', false },
	{ 'd', false },
	{ ' ', false },
	{ '\\', false}
};

const unsigned int simulationTimePrecision = 1000; // the lower, the better;
const int simulationSpeedChangeRatio = 10;
const unsigned int deltaT = 16;
const double scale = 1/5e8;
const int minCamSpeed = 1;
const int maxCamSpeed = 200;
const double mouseSensitivity = 0.05;
const int gridSize = 5000;
const int gridSpacement = 50;

Body sun, earth;
GLfloat fov = 45, fAspect, largura = 1200, altura = 900, yaw = -90, pitch = 0;
Coordinates camera{ 0, 0, 0 }, lookAtHim{ camera.x, camera.y, camera.z-1 };
int camSpeed = 10;
int previousMouseX, previousMouseY;
int simulationSpeed = 25;
bool simulationPaused = false;

void LogCoordinates(Coordinates coordinates) {
	cout << "x: " << coordinates.x << endl;
	cout << "y: " << coordinates.y << endl;
	cout << "z: " << coordinates.z << endl;
}

// Function to calculate gravitational force between two bodies
void CalculateGravity(Body& body1, Body& body2, double& fx, double& fz) {
    double dx = body2.x - body1.x;
    double dy = body2.z - body1.z;
    double r = sqrt(dx * dx + dy * dy);

    double F = (G * body1.mass * body2.mass) / (r * r);

    fx = F * (dx / r);
    fz = F * (dy / r);
}

// Function to update the position and velocity of a body based on forces
void UpdateBody(Body& body, double fx, double fz, int dt) {
    double ax = fx / body.mass;
    double az = fz / body.mass;

    body.vx += ax * dt;
    body.vz += az * dt;

    body.x += body.vx * dt;
    body.z += body.vz * dt;
}

void DrawCrosshair() {
	glColor3f(0, 1, 1);
	glPushMatrix();
        glTranslated(lookAtHim.x, lookAtHim.y, lookAtHim.z);
		glutWireSphere(0.001, 10, 10);
    glPopMatrix();
}

void DrawBodies() {
	glColor3f(1.0f, 0.5f, 0.0f);
    glPushMatrix();
        glTranslated(0, 0, 0);
        glutSolidSphere(50, 50, 20);
    glPopMatrix();

	glColor3f(0, 0.5f, 0.3f);
    glPushMatrix();
        glTranslated(earth.x*scale, 0, earth.z*scale);
        glutSolidSphere(5, 20, 20);
    glPopMatrix();
}

void DrawStar() {
	double brightness = 0.5/(rand()+1) + 0.5;
	double angle = rand();
	glPushMatrix();
		glRotated(angle, camera.x, camera.y, camera.z);
        glTranslated(lookAtHim.x+10, lookAtHim.y+10, lookAtHim.z+10);
		glutWireSphere(0.001, 10, 10);
    glPopMatrix();
}

void DrawStars() {
	
}

// Adapted from https://3dengine.org/Draw_a_grid/
void DrawXZPlaneGrid() {
	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_LINES);
	for(int i = -gridSize; i <= gridSize; i += gridSpacement) {
	    glVertex3f(i,0,-gridSize);
	    glVertex3f(i,0,gridSize);
	    glVertex3f(-gridSize,0,i);
	    glVertex3f(gridSize,0,i);
	};
	glEnd();
}

void Desenha(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, largura, altura);

	DrawCrosshair();
	DrawXZPlaneGrid();
	// DrawStar();
	DrawBodies();

	glutSwapBuffers();
}

void Inicializa(void)
{
	/*
	GLfloat luzAmbiente[4]={0.2, 0.2, 0.2, 1.0};   // {R, G, B, alfa}
	GLfloat luzDifusa[4]={0.5, 0.5, 0.5, 1.0};	   // o 4o componente, alfa, controla a opacidade/transparência da luz
	GLfloat luzEspecular[4]={1.0, 1.0, 1.0, 1.0};
	GLfloat posicaoLuz[4]={50.0, 50.0, 50.0, 1.0};  // aqui o 4o componente indica o tipo de fonte:
                                                    // 0 para luz direcional (no infinito) e 1 para luz pontual (em x, y, z)

	// Capacidade de brilho do material
	GLfloat especularidade[4]={1.0, 1.0, 1.0, 1.0}; 
	GLint especMaterial = 100;

 	// Especifica a cor de fundo da janela
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	// Habilita o modelo de colorização de Gouraud
	glShadeModel(GL_SMOOTH);  // a cor de cada ponto da primitiva é interpolada a partir dos vértices
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

    glEnable(GL_DEPTH_TEST);   //ativa o zBuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   //aplica o zBuffer
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
	const int camSpeedChangeRatio = 2;
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
}

bool isMovementKey(unsigned char key) {
	return movementKeyPressed.find(key) != movementKeyPressed.end();
}

void KeyboardFunc(unsigned char key, int x, int y) {
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
	default:
		break;
	}
}

void KeyboardUpFunc(unsigned char key, int x, int y) {
	if (isMovementKey(key)) {
		movementKeyPressed.at(key) = false;
	}
}

void SimulationTick() {
	int absSimulationSpeed = abs(simulationSpeed);
	// timeWay: -1 (backwards in time) or 1 (forwards in time)
	int timeWay = absSimulationSpeed/simulationSpeed;

	for (int t = 0; t < absSimulationSpeed; t++) {
        double fx, fz;

        CalculateGravity(earth, sun, fx, fz);
        UpdateBody(earth, fx, fz, timeWay*simulationTimePrecision);
    }
}

void UpdateMovement() {
	if (movementKeyPressed.at('w')) {
		UpdateCamera(false, POSITIVE);
	}
	if (movementKeyPressed.at('a')) {
		UpdateCamera(true, NEGATIVE);
	}
	if (movementKeyPressed.at('s')) {
		UpdateCamera(false, NEGATIVE);
	}
	if (movementKeyPressed.at('d')) {
		UpdateCamera(true, POSITIVE);
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

void Timer(int _ = 0) {
	UpdateMovement();
	if (simulationSpeed != 0 && !simulationPaused) {
		SimulationTick();
	}

	EspecificaParametrosVisualizacao();
    glutPostRedisplay();
    glutTimerFunc(deltaT, Timer, _);
}

void SetBodies() {
    // ifstream inputFile("Data/data.txt");
	// double _;

    // inputFile >> simulationSpeed >> _;
    // inputFile >> sun.mass >> sun.x >> sun.y >> sun.vx >> sun.vy;
    // inputFile >> earth.mass >> earth.x >> earth.y >> earth.vx >> earth.vy;

	sun.mass = 1.989e30;
	sun.x = 0;
	sun.z = 0;
	sun.vx = 0;
	sun.vz = 0;

	earth.mass = 5.972e24;
	earth.x = 147e9;
	earth.z = 0;
	earth.vx = 0;
	earth.vz = 29783;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(0,0);
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
