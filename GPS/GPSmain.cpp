
#include "GPS.h"
#include "SMStructs.h"
#include <SMObject.h>

#include <iostream>
#include <conio.h>

#define TIMEOUT 1000

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace Net;
using namespace Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {
	

	GPS GPSModule;
	GPSModule.setupSharedMemory();
	
	/*
	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagement* PMData = NULL;

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("ERROR: Process Management SM Object not accessed");
	}

	PMData = (ProcessManagement*)PMObj.pData;
	*/

	double WaitTimeGPS = 0.00;
	
	while (!GPSModule.getShutdownFlag()) {

		//GPSModule.checkData();
		GPSModule.setHeartbeat(1);
	}

	GPSModule.~GPS();
	

	/*
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

		Thread::Sleep(100);
		
		if (PMData->Shutdown.Status) {
			break;
		}

	}
	*/

	return 0;
}