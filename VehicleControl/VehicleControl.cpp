#include "SMStructs.h"
#include "SMObject.h"

#include <iostream>
#include <conio.h>

#define TIMEOUT 1000

using namespace System::Threading;
using namespace System;

int VCShutdown = 0;
double WaitTimeVC = 0.00;


int main() {

	/*
	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagement* PMData = NULL;

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("ERROR: Process Management SM Object not accessed");
	}

	PMData = (ProcessManagement*)PMObj.pData;

	while (!PMData->Shutdown.Flags.VehicleControl)
	{
		if (PMData->Heartbeat.Flags.VehicleControl == 0) {
			PMData->Heartbeat.Flags.VehicleControl = 1;
			WaitTimeVC = 0.00;
		}
		else {
			WaitTimeVC += 25;
			if (WaitTimeVC > TIMEOUT) {
				PMData->Shutdown.Status = 0xFF;
			}
		}

		Thread::Sleep(25);

		if (PMData->Shutdown.Status) {
			break;
		}

	}
	*/

	return 0;
}