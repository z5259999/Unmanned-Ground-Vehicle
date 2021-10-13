
#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "SMStructs.h"
#include "SMObject.h"

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

#define NUM_UNITS 5

bool IsProcessRunning(const char* processName);
void StartProcesses();

//defining start up sequence
TCHAR Units[10][20] = //
{
	TEXT("Laser.exe"),
	TEXT("VehicleControl.exe"),
	TEXT("Display.exe"),
	TEXT("GPS.exe"),
	TEXT("Camera.exe"),
	//TEXT("OpenGL.exe")
};

struct waitAndSee {

	double Laser = 0.00;
	double VC = 0.00;
	double Display = 0.00;
	double GPS = 0.00;
	double Camera = 0.00;	
};

int main()
{
	
	// Create struct to keep the waitAndSee times
	waitAndSee waitTime = {0.00, 0.00, 0.00, 0.00, 0.00};

	

	///////////////////////////PM///////////////////////////////////////

	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagement* PMData = NULL;
	
	PMObj.SMCreate();
	if(PMObj.SMCreateError) {
		Console::WriteLine("ERROR: Process Management SM Object not created");
	}
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("ERROR: Process Management SM Object not accessed");
	}

	PMData = (ProcessManagement*)PMObj.pData;

	//start all 5 modules
	StartProcesses();

	/*
	///////////////////////////LASER///////////////////////////////////

	SMObject LaserObj(_TEXT("Laser"), sizeof(SM_Laser));
	SM_Laser* LaserData = NULL;

	LaserObj.SMCreate();
	if (LaserObj.SMCreateError) {
		Console::WriteLine("ERROR: Laser SM Object not created");
	}
	LaserObj.SMAccess();
	if (LaserObj.SMAccessError) {
		Console::WriteLine("ERROR: Laser SM Object not accessed");
	}

	LaserData = (SM_Laser*)LaserObj.pData;


	///////////////////////Vehicle Control////////////////////////////

	SMObject VCObj(_TEXT("VehicleControl"), sizeof(SM_VehicleControl));
	SM_VehicleControl* VCData = NULL;

	VCObj.SMCreate();
	if (VCObj.SMCreateError) {
		Console::WriteLine("ERROR: Vehicle Control SM Object not created");
	}
	VCObj.SMAccess();
	if (VCObj.SMAccessError) {
		Console::WriteLine("ERROR: Vehicle Control SM Object not accessed");
	}

	VCData = (SM_VehicleControl*)VCObj.pData;


	///////////////////////////GPS////////////////////////////////////

	SMObject GPSObj(_TEXT("GPS"), sizeof(SM_GPS));
	SM_GPS* GPSData = NULL;

	GPSObj.SMCreate();
	if (GPSObj.SMCreateError) {
		Console::WriteLine("ERROR: GPS SM Object not created");
	}
	GPSObj.SMAccess();
	if (GPSObj.SMAccessError) {
		Console::WriteLine("ERROR: GPS SM Object not accessed");
	}

	GPSData = (SM_GPS*)GPSObj.pData;

	//////////////////////////////////////////////////////////////////
	*/

	PMData->Shutdown.Status = 0x00;
	double WaitAndSee = 0.00;
	while (!_kbhit()) {

		
		///////////////////////////LASER///////////////////////////////////
		// Heartbeats: Laser CRITICAL
		if (PMData->Heartbeat.Flags.Laser == 1) {
			PMData->Heartbeat.Flags.Laser = 0;
			waitTime.Laser = 0.00;
			//std::cout << "Laser Running " << waitTime.Laser << std::endl;
		}
		else {
			waitTime.Laser += 25;
			if (waitTime.Laser > TIMEOUT) {
				PMData->Shutdown.Flags.Laser = 1;
				//std::cout << "Laser Dead " << waitTime.Laser << std::endl;
				//break;
			}
		}
		
		//////////////////////Vehicle Control//////////////////////////////
		// Heartbeats: VC CRITICAL
		if (PMData->Heartbeat.Flags.VehicleControl == 1) {
			PMData->Heartbeat.Flags.VehicleControl = 0;
			waitTime.VC = 0.00;
		}
		else {
			waitTime.VC += 25;
			if (waitTime.VC > TIMEOUT) {
				//PMData->Shutdown.Flags.VehicleControl = 1;
				//std::cout << "Yes this sucks lol - VC" << waitTime.VC << std::endl;
				std::cout << "VC UGH" << waitTime.VC << std::endl;
				//break;
			}
		}
		

		///////////////////////////DISPLAY///////////////////////////////////
		// Heartbeats: Display CRITICAL
		if (PMData->Heartbeat.Flags.Display == 1) {
			PMData->Heartbeat.Flags.Display = 0;
			WaitAndSee = 0.00;
		}
		else {
			waitTime.Display += 25;
			if (waitTime.Display > TIMEOUT) {
				std::cout << "Display UGH " << waitTime.Display << std::endl;
				PMData->Shutdown.Status = 0xFF;
				//break;
			}
		}


		///////////////////////////GPS///////////////////////////////////////
		// Heartbeats: GPS NONCRITICAL
		if (PMData->Heartbeat.Flags.GPS == 1) {
			PMData->Heartbeat.Flags.GPS = 0;
			waitTime.GPS = 0.00;
		}
		else {
			waitTime.GPS += 25;
			if (waitTime.GPS > TIMEOUT) {
				//std::cout << "RESTARTING GPS: " << waitTime.GPS << std::endl;
				std::cout << "GPS UGH" << waitTime.GPS << std::endl;
				StartProcesses();
			}
		}
		

		///////////////////////////Camera///////////////////////////////////////
		// Heartbeats: Camera CRITICAL
		if (PMData->Heartbeat.Flags.Camera == 1) {
			PMData->Heartbeat.Flags.Camera = 0;
			waitTime.Camera = 0.00;
			//std::cout << "RESET- CAMERA" << waitTime.Camera<< std::endl;
		}
		else {
			waitTime.Camera += 25;
			if (waitTime.Camera > TIMEOUT) {
				//std::cout << "RESTARTING CAMERA: " << waitTime.Camera << std::endl;
				Console::WriteLine("big sad");
				StartProcesses();
			}
		}

		Sleep(25);

	}
	
	//shutdown after main loop exits
	PMData->Shutdown.Status = 0xFF;

	return 0;
}


//Is process running function
bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp((const char *)entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}


void StartProcesses()
{
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	for (int i = 0; i < NUM_UNITS; i++)
	{
		if (!IsProcessRunning((const char *)Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));

			if (!CreateProcess(NULL, Units[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &s[i], &p[i]))
			{
				printf("%s failed (%d).\n", Units[i], GetLastError());
				_getch();
			}
			std::cout << "Started: " << Units[i] << std::endl;
			Sleep(100);
		}
	}
}

