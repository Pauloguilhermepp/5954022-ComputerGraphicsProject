#include <GL/glut.h>
#include <iostream>
#include <cmath>

#define POSITIVE 1
#define NEGATIVE -1
#define PI 3.14159265358

using namespace std;

struct Coordinates {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

void logCoordinates(Coordinates coordinates) {
	cout << "x: " << coordinates.x << endl;
	cout << "y: " << coordinates.y << endl;
	cout << "z: " << coordinates.z << endl;
}

const int camAngleChangeRatio = 1;
const int scrollChangeRatio = 2;
const int minCamSpeed = 1;
const int maxCamSpeed = 200;
const double mouseSensitivity = 0.1;

GLfloat fov, fAspect, largura, altura, yaw = -90, pitch = 0;
Coordinates camera{ 0, 0, 0 }, lookAtHim{ camera.x, camera.y, camera.z-1 };
int camSpeed = 10;
int previousMouseX, previousMouseY;

void Desenha(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, largura, altura);

	glColor3f(0, 0, 0);
	glPushMatrix();
        glTranslated(lookAtHim.x, lookAtHim.y, lookAtHim.z);
		glutWireSphere(0.001, 10, 10);
    glPopMatrix();
	
    glColor3f(1.0f, 0.2f, 0.2f);
    glPushMatrix();
        glTranslated(0, 0, -250);
        glutWireTeapot(50.0f);
    glPopMatrix();

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

int main(int argc, char** argv)
{
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

	glutDisplayFunc(Desenha);
	glutReshapeFunc(AlteraTamanhoJanela); // Função para ajustar o tamanho da tela
    glutMouseFunc(GerenciaCliqueMouse);
    glutKeyboardFunc(GerenciaTeclado); // Define qual funcao gerencia o comportamento do teclado
    glutSpecialFunc(TeclasEspeciais); // Define qual funcao gerencia as teclas especiais
	glutPassiveMotionFunc(GerenciaMovimentoMouse);
	Inicializa();
	glutMainLoop();

}
