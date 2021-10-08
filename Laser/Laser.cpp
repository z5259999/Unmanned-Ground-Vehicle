
#include "SMStructs.h"
#include "SMObject.h"

#include <iostream>
#include <conio.h>

#define TIMEOUT 1000

using namespace System::Threading;
using namespace System;

int LaserShutdown = 0;
double WaitTimeLaser = 0.00;


int main() {

	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagement* PMData = NULL;

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("ERROR: Process Management SM Object not accessed");
	}

	PMData = (ProcessManagement*)PMObj.pData;

	while (!PMData->Shutdown.Flags.Laser)
	{
		if (PMData->Heartbeat.Flags.Laser == 0) {
			PMData->Heartbeat.Flags.Laser = 1;
			WaitTimeLaser = 0.00;
		}
		else {
			WaitTimeLaser += 25;
			if (WaitTimeLaser > TIMEOUT) {
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