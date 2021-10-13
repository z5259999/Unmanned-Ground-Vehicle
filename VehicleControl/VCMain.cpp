#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "SMStructs.h"
#include "SMObject.h"
#include "VehicleControl.h"

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace Net;
using namespace Sockets;
using namespace System::Net;
using namespace System::Text;


int main() 
{
	/*
	VehicleControl VCModule;
	VCModule.setupSharedMemory();

	while (!VCModule.getShutdownFlag()) {

		VCModule.setHeartbeat(1);
	}

	VCModule.~VehicleControl();
	*/

	double WaitTimeVC = 0.00;
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

		Thread::Sleep(100);

		if (PMData->Shutdown.Status) {
			break;
		}

	}
	


	return 0;
}