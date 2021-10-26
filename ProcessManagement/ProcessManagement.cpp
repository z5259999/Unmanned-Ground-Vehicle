
// Credited Authors: Jay Katupitiya, James Stevens
// Codebase (Heartbeats) derived from week 4 Pre-recorded videos [Accessed from 1 Oct 2021] 
// Available: https://moodle.telt.unsw.edu.au/mod/resource/view.php?id=4062689


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
	TEXT("Laser33.exe"),
	TEXT("VehicleControl4.exe"),
	TEXT("Display.exe"),
	TEXT("GPS.exe"),
	TEXT("Camera.exe"),
};

int main()
{
	
	// Create array to keep the waitAndSee times
	double waitTime[5] = {0.00, 0.00, 0.00, 0.00, 0.00};

	
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

	//start all 5 modules
	StartProcesses();
	PMData->Shutdown.Status = 0x00;

	while (!_kbhit()) {

		
		///////////////////////////LASER///////////////////////////////////
		
		// Heartbeats: Laser CRITICAL
		// Flip the flag from 1 to 0 - Communicates with Laser.cpp
		if (PMData->Heartbeat.Flags.Laser == 1) {
			PMData->Heartbeat.Flags.Laser = 0;
			waitTime[TIME_LASER] = 0.00;
			Console::WriteLine("Laser OK");
		}
		else {
			waitTime[TIME_LASER] += 25;
			// No response for CRITICAL PROCESS - Kill Program
			if (waitTime[TIME_LASER] > TIMEOUT) {
				Console::WriteLine("Laser NOT OK: CRITICAL SHUTDOWN");
				PMData->Shutdown.Status = 0xFF;
				//break;
			}
		}
		
		//////////////////////Vehicle Control//////////////////////////////
		// Heartbeats: VC CRITICAL
		if (PMData->Heartbeat.Flags.VehicleControl == 1) {
			PMData->Heartbeat.Flags.VehicleControl = 0;
			waitTime[TIME_VC] = 0.00;
		}
		else {
			waitTime[TIME_VC] += 25;
			if (waitTime[TIME_VC] > TIMEOUT) {
				PMData->Shutdown.Status = 0xFF;
				Console::WriteLine("VC NOT OK: CRITICAL SHUTDOWN");
				break;
			}
		}
		

		///////////////////////////DISPLAY///////////////////////////////////
		// Heartbeats: Display NONCRITICAL
		if (PMData->Heartbeat.Flags.Display == 1) {
			PMData->Heartbeat.Flags.Display = 0;
			waitTime[TIME_DISP] = 0.00;
		}
		else {
			waitTime[TIME_DISP] += 25;
			if (waitTime[TIME_DISP] > TIMEOUT) {
				Console::WriteLine("Display NOT OK: RESTARTING");
				StartProcesses();
				waitTime[TIME_DISP] = 0.00;
			}
		}


		///////////////////////////GPS///////////////////////////////////////
		// Heartbeats: GPS NONCRITICAL
		if (PMData->Heartbeat.Flags.GPS == 1) {
			PMData->Heartbeat.Flags.GPS = 0;
			waitTime[TIME_GPS] = 0.00;
		}
		else {
			waitTime[TIME_GPS] += 25;
			if (waitTime[TIME_GPS] > TIMEOUT) {
				Console::WriteLine("GPS NOT OK: RESTARTING");
				StartProcesses();
				waitTime[TIME_GPS] = 0.00;
			}
		}
		

		///////////////////////////Camera///////////////////////////////////////
		// Heartbeats: Camera CRITICAL
		if (PMData->Heartbeat.Flags.Camera == 1) {
			PMData->Heartbeat.Flags.Camera = 0;
			waitTime[TIME_CAMERA] = 0.00;
		}
		else {
			waitTime[TIME_CAMERA] += 25;
			if (waitTime[TIME_CAMERA] > TIMEOUT) {
				Console::WriteLine("CAMERA NOT OK: CRITICAL SHUTDOWN");
				PMData->Shutdown.Status = 0xFF;
				break;
			}
		}

		Sleep(50);
		
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

