
#include "GPS.h"
#include "SMStructs.h"
#include <SMObject.h>

#include <iostream>
#include <conio.h>

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace Net;
using namespace Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {
	
	/*
	GPS GPSModule;
	GPSModule.setupSharedMemory();
	
	while (!GPSModule.getShutdownFlag()) {

		//GPSModule.checkData();
		GPSModule.setHeartbeat(1);
	}

	GPSModule.~GPS();
	*/

	double WaitTimeGPS = 0.00;
	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagement* PMData = NULL;

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("ERROR: Process Management SM Object not accessed");
	}

	PMData = (ProcessManagement*)PMObj.pData;
	

	while (!PMData->Shutdown.Flags.GPS)
	{
		if (PMData->Heartbeat.Flags.GPS == 0) {
			PMData->Heartbeat.Flags.GPS = 1;
			WaitTimeGPS = 0.00;
		}
		else {
			WaitTimeGPS += 25;
			if (WaitTimeGPS > TIMEOUT) {

				PMData->Shutdown.Status = 0xFF;
			}
		}

		Thread::Sleep(25);
		
		if (PMData->Shutdown.Status) {
			exit(0);
			break;
		}

	}

	return 0;
}