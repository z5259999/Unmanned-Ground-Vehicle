
#using <System.dll>
#include <conio.h>

//paste all the SMObject into the source files folder for each of the sections
#include <SMObject.h>
#include <smstructs.h>	


#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <unistd.h>
#include <sys/time.h>
#elif defined(WIN32)
#include <Windows.h>
#include <tchar.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <unistd.h>
#include <sys/time.h>
#endif


#include "Camera.hpp"
#include "Ground.hpp"
#include "KeyManager.hpp"

#include "Shape.hpp"
#include "Vehicle.hpp"
#include "MyVehicle.hpp"

#include "Messages.hpp"
#include "HUD.hpp"

#define DEGTORAD (3.141592765 / 180.0)


void display();
void reshape(int width, int height);
void idle();

void keydown(unsigned char key, int x, int y);
void keyup(unsigned char key, int x, int y);
void special_keydown(int keycode, int x, int y);
void special_keyup(int keycode, int x, int y);

void mouse(int button, int state, int x, int y);
void dragged(int x, int y);
void motion(int x, int y);
void drawLaser();
void drawGPS();

using namespace std;
using namespace scos;
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;


// Used to store the previous mouse location so we
//   can calculate relative mouse movement.
int prev_mouse_x = -1;
int prev_mouse_y = -1;

// vehicle control related variables
Vehicle* vehicle = NULL;
double speed = 0;
double steering = 0;

SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
SMObject LaserSMObject(TEXT("LaserSMObject"), sizeof(SM_Laser));
SMObject GPSSMObject(TEXT("GPSSMObject"), sizeof(SM_GPS));
SMObject VehicleSMObject(TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));
ProcessManagement* PMData = nullptr;
SM_Laser* LaserData = nullptr;
SM_GPS* GPSData = nullptr;
SM_VehicleControl* VehicleData = nullptr;

//int _tmain(int argc, _TCHAR* argv[]) {
int main(int argc, char** argv) {

	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;
	//Console::WriteLine("set up shared memory");
	// 
	//PMObj.SMCreate();
	//LaserSMObject.SMCreate();
	//GPSSMObject.SMCreate();
	//VehicleSMObject.SMCreate();

	PMObj.SMAccess();

	LaserSMObject.SMAccess();

	GPSSMObject.SMAccess();

	VehicleSMObject.SMAccess();

	PMData = (ProcessManagement*)PMObj.pData;
	LaserData = (SM_Laser*)LaserSMObject.pData;
	GPSData = (SM_GPS*)GPSSMObject.pData;
	VehicleData = (SM_VehicleControl*)VehicleSMObject.pData;

	PMData->Shutdown.Flags.Display = 0;

	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - GL");

	Camera::get()->setWindowDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(special_keydown);
	glutSpecialUpFunc(special_keyup);

	glutMouseFunc(mouse);
	glutMotionFunc(dragged);
	glutPassiveMotionFunc(motion);

	// -------------------------------------------------------------------------
	// Please uncomment the following line of code and replace 'MyVehicle'
	//   with the name of the class you want to show as the current 
	//   custom vehicle.
	// -------------------------------------------------------------------------
	vehicle = new MyVehicle();


	glutMainLoop();

	if (vehicle != NULL) {
		delete vehicle;
	}

	return 0;
}


void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (Camera::get()->isPursuitMode() && vehicle != NULL) {
		double x = vehicle->getX(), y = vehicle->getY(), z = vehicle->getZ();
		double dx = cos(vehicle->getRotation() * 3.141592765 / 180.0);
		double dy = sin(vehicle->getRotation() * 3.141592765 / 180.0);
		Camera::get()->setDestPos(x + (-3 * dx), y + 7, z + (-3 * dy));
		Camera::get()->setDestDir(dx, -1, dy);
	}
	Camera::get()->updateLocation();
	Camera::get()->setLookAt();

	Ground::draw();

	// draw my vehicle
	if (vehicle != NULL) {
		vehicle->draw();

	}

	drawLaser();
	drawGPS();

	// draw HUD
	HUD::Draw();

	glutSwapBuffers();
};

void drawLaser() {
	

	glPushMatrix();
	glBegin(GL_LINES);
	// For all Laser data points, render a line in their place
	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		
		// Set to white lines of 2 thickness (pixels?)
		glColor3f(1, 1, 1);
		glLineWidth(2);

		// Divided by 1000 to get m, -y to reflect the demo code
		double xPos = (LaserData->x[i] / 1000);
		double yPos = -(LaserData->y[i] / 1000);

		// Create one vertext at xPos, yPos, then create second point 1 unit above to make line
		glVertex3f(xPos, 0, yPos);
		glVertex3f(xPos, 1, yPos);

	}

	glEnd();
	glPopMatrix();

}

void drawGPS() {

	// Render text in 2D space (On screen)
	Camera::get()->switchTo2DDrawing();
	
	// Position Finding
	int winWidthOff = (Camera::get()->getWindowWidth() - 800) * .5;
	
	// If the window is too small, print in top left
	if (winWidthOff < 0) {
		winWidthOff = 0;
	}
	
	char buffer[80];
	if (vehicle) {
		
		//Print values in white text on screen in Helvetica
		glColor3f(1, 1, 1);
		sprintf(buffer, "Northing: % .4f  Easting: % .4f  Height: % .4f", GPSData->northing,
			GPSData->easting, GPSData->height);
		HUD::RenderString(buffer, 0, 20, GLUT_BITMAP_HELVETICA_12);
	}

	// Finish in 2D rendering, go to 3D space to draw other elements
	Camera::get()->switchTo3DDrawing();

}


void reshape(int width, int height) {

	Camera::get()->setWindowDimensions(width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
};

double getTime()
{
#if defined(WIN32)
	LARGE_INTEGER freqli;
	LARGE_INTEGER li;
	if (QueryPerformanceCounter(&li) && QueryPerformanceFrequency(&freqli)) {
		return double(li.QuadPart) / double(freqli.QuadPart);
	}
	else {
		static ULONGLONG start = GetTickCount64();
		return (GetTickCount64() - start) / 1000.0;
	}
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + (t.tv_usec / 1000000.0);
#endif
}

// put code in here
void idle() {


	double WaitAndSee = 0.00;

	if (PMData->Heartbeat.Flags.Display == 0) {
		PMData->Heartbeat.Flags.Display = 1;

		//Debugging
		//Console::WriteLine("HB Display: " + PMData->Heartbeat.Flags.Display);

		WaitAndSee = 0.00;
	}
	else {
		WaitAndSee += 25;
		if (WaitAndSee > TIMEOUT) {
			PMData->Shutdown.Status = 0xFF;
		}
	}
	Thread::Sleep(25);

	if (PMData->Shutdown.Status) {
		exit(0);
	}

	if (KeyManager::get()->isAsciiKeyPressed('a')) {
		Camera::get()->strafeLeft();
	}

	if (KeyManager::get()->isAsciiKeyPressed('c')) {
		Camera::get()->strafeDown();
	}

	if (KeyManager::get()->isAsciiKeyPressed('d')) {
		Camera::get()->strafeRight();
	}

	if (KeyManager::get()->isAsciiKeyPressed('s')) {
		Camera::get()->moveBackward();
	}

	if (KeyManager::get()->isAsciiKeyPressed('w')) {
		Camera::get()->moveForward();
	}

	if (KeyManager::get()->isAsciiKeyPressed(' ')) {
		Camera::get()->strafeUp();
	}

	speed = 0;
	steering = 0;

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_LEFT)) {
		steering = Vehicle::MAX_LEFT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
		steering = Vehicle::MAX_RIGHT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_UP)) {
		speed = Vehicle::MAX_FORWARD_SPEED_MPS;

	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_DOWN)) {
		speed = Vehicle::MAX_BACKWARD_SPEED_MPS;
	}

	VehicleData->Steering = steering;
	VehicleData->Speed = speed;

	const float sleep_time_between_frames_in_seconds = 0.025;

	static double previousTime = getTime();
	const double currTime = getTime();
	const double elapsedTime = currTime - previousTime;
	previousTime = currTime;

	// do a simulation step
	if (vehicle != NULL) {
		vehicle->update(speed, steering, elapsedTime);
	}

	display();

#ifdef _WIN32 
	Sleep(sleep_time_between_frames_in_seconds * 1000);
#else
	usleep(sleep_time_between_frames_in_seconds * 1e6);
#endif
};

void keydown(unsigned char key, int x, int y) {

	// keys that will be held down for extended periods of time will be handled
	//   in the idle function
	KeyManager::get()->asciiKeyPressed(key);

	// keys that react ocne when pressed rather than need to be held down
	//   can be handles normally, like this...
	switch (key) {
	case 27: // ESC key
		exit(0);
		break;
	case '0':
		Camera::get()->jumpToOrigin();
		break;
	case 'p':
		Camera::get()->togglePursuitMode();
		break;
	}

};

void keyup(unsigned char key, int x, int y) {
	KeyManager::get()->asciiKeyReleased(key);
};

void special_keydown(int keycode, int x, int y) {

	KeyManager::get()->specialKeyPressed(keycode);

};

void special_keyup(int keycode, int x, int y) {
	KeyManager::get()->specialKeyReleased(keycode);
};

void mouse(int button, int state, int x, int y) {

};

void dragged(int x, int y) {

	if (prev_mouse_x >= 0) {

		int dx = x - prev_mouse_x;
		int dy = y - prev_mouse_y;

		Camera::get()->mouseRotateCamera(dx, dy);
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
};

void motion(int x, int y) {

	prev_mouse_x = x;
	prev_mouse_y = y;
};


