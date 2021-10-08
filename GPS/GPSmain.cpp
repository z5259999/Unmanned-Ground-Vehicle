
#include "GPS.h"
#include "SMStructs.h"
#include "SMObject.h"

#include <iostream>
#include <conio.h>

#define TIMEOUT 1000

using namespace System::Threading;
using namespace System;

int GPSShutdown = 0;
double WaitTimeGPS = 0.00;


int main() {
	
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
			break;
		}

	}

	return 0;
}