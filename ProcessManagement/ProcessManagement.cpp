
#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

//paste all the SMObject into the source files folder for each of the sections
#include <SMObject.h>
#include <smstructs.h>	



using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

#define NUM_UNITS 5

bool IsProcessRunning(const char* processName);
void StartProcesses();

TCHAR Units[10][20] = //
{
	TEXT("LaserMod.exe"),
	TEXT("VehicleMod.exe"),
	TEXT("DisplayMod.exe"),
	TEXT("GPSMod.exe"),
	TEXT("CameraMod.exe")
};


int main() {

	// Shared Memory Creation and Access

	//////////////////////////////PM////////////////////////////////////
	
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));

	PMObj.SMCreate();
	if (PMObj.SMCreateError) {
		Console::WriteLine("ERROR: Process Management SM Object not created");
	}
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("ERROR: Process Management SM Object not accessed");
	}

	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	///////////////////////////LASER///////////////////////////////////
	
	SMObject LaserSMObject(TEXT("LaserSMObject"), sizeof(SM_Laser));

	LaserSMObject.SMCreate();
	if (LaserSMObject.SMCreateError) {
		Console::WriteLine("ERROR: Laser SM Object not created");
	}
	LaserSMObject.SMAccess();
	if (LaserSMObject.SMAccessError) {
		Console::WriteLine("ERROR: Laser SM Object not accessed");
	}

	SM_Laser* LaserData = (SM_Laser*)LaserSMObject.pData;

	///////////////////////////Vehicle/////////////////////////////////
	
	SMObject VehicleSMObject(TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));

	VehicleSMObject.SMCreate();
	if (VehicleSMObject.SMCreateError) {
		Console::WriteLine("ERROR: Vehicle Control SM Object not created");
	}
	VehicleSMObject.SMAccess();
	if (VehicleSMObject.SMAccessError) {
		Console::WriteLine("ERROR: Vehicle Control SM Object not accessed");
	}

	SM_VehicleControl* VehicleData = (SM_VehicleControl*)VehicleSMObject.pData;

	/////////////////////////////GPS///////////////////////////////////
	
	SMObject GPSSMObject(TEXT("GPSSMObject"), sizeof(SM_GPS));

	GPSSMObject.SMCreate();
	if (GPSSMObject.SMCreateError) {
		Console::WriteLine("ERROR: GPS SM Object not created");
	}
	GPSSMObject.SMAccess();
	if (GPSSMObject.SMAccessError) {
		Console::WriteLine("ERROR: GPS SM Object not accessed");
	}

	SM_GPS* GPSData = (SM_GPS*)GPSSMObject.pData;


	///////////////////////////////////////////////////////////////////

	PMData->PMHeartbeat.Status = 0xFF;

	StartProcesses();

	int LaserCounter = 0;
	int DisplayCounter = 0;
	int GPSCounter = 0;
	int VehicleCounter = 0;
	int CameraCounter = 0;

	double waitTime[5] = { 0.00, 0.00, 0.00, 0.00, 0.00 };

	while (!_kbhit()) {

		PMData->PMHeartbeat.Status = 0xFF;

		///////////////////////////LASER///////////////////////////////////
		// Heartbeats: Laser CRITICAL
		if (PMData->Heartbeat.Flags.Laser == 1) {
			LaserCounter = 0;
			PMData->Heartbeat.Flags.Laser = 0;
			//Console::WriteLine("HB Laser: " + PMData->Heartbeat.Flags.Laser);
		}
		else {
			if (LaserCounter > TIMEOUT) {
				Console::WriteLine("LASER NOT OK: CRITICAL SHUTDOWN");
				PMData->Shutdown.Status = 0xFF;
				break;
			}
			else {
				waitTime[WAIT_LASER]++;
			}


		}


		///////////////////////////DISPLAY///////////////////////////////////
		// Heartbeats: Display NONCRITICAL
		if (PMData->Heartbeat.Flags.Display == 1) {
			DisplayCounter = 0;
			PMData->Heartbeat.Flags.Display = 0;
		}
		else {
			if (DisplayCounter > TIMEOUT) {
				Console::WriteLine("Display NOT OK: RESTARTING");
				StartProcesses();
			}
			else {
				waitTime[WAIT_DISPLAY]++;
			}

		}


		//////////////////////Vehicle Control//////////////////////////////
		// Heartbeats: VC CRITICAL
		if (PMData->Heartbeat.Flags.VehicleControl == 1) {
			VehicleCounter = 0;
			PMData->Heartbeat.Flags.VehicleControl = 0;
		}
		else {
			if (VehicleCounter > TIMEOUT) {
				Console::WriteLine("VC NOT OK : CRITICAL SHUTDOWN");
				PMData->Shutdown.Status = 0xFF;
				break;
			}
			else {
				waitTime[WAIT_VEHICLE]++;
			}

		}


		///////////////////////////GPS///////////////////////////////////////
		// Heartbeats: GPS NONCRITICAL
		if (PMData->Heartbeat.Flags.GPS == 1) {
			GPSCounter = 0;
			PMData->Heartbeat.Flags.GPS = 0;
			//Console::WriteLine("HB GPS: " + PMData->Heartbeat.Flags.GPS);
		}
		else {
			if (GPSCounter > TIMEOUT) {
				Console::WriteLine("GPS NOT OK: RESTARTING");
				StartProcesses();
			}
			else {
				waitTime[WAIT_GPS]++;
			}

		}


		///////////////////////////Camera///////////////////////////////////////
		// Heartbeats: Camera CRITICAL
		if (PMData->Heartbeat.Flags.Camera == 1) {
			CameraCounter = 0;
			PMData->Heartbeat.Flags.Camera = 0;
		}
		else {
			// checks if wait time has elapsed
			if (CameraCounter > TIMEOUT) {
				Console::WriteLine("CAMERA NOT OK: RESTARTING");
				StartProcesses();
			}
			else {
				waitTime[WAIT_CAMERA]++;
			}

		}

		Thread::Sleep(50);

	}

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
			if (!_stricmp((const char*)entry.szExeFile, processName))
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
		if (!IsProcessRunning((const char*)Units[i]))
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


