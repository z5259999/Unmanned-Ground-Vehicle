#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "SMStructs.h"
#include "SMObject.h"
#include "Laser.h"

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace Net;
using namespace Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {
	
	String^ hostName = "192.168.1.200";
	int portNumber = 23000;

	Laser LaserModule;
	LaserModule.setupSharedMemory();
	LaserModule.connect(hostName, portNumber);

	while (!LaserModule.getShutdownFlag()) {

		LaserModule.checkData();
		LaserModule.setHeartbeat(1);
	}
	
	LaserModule.~Laser();
	
	return 0;
	
	
	/*
	///////////////////////////PM///////////////////////////////////////

	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagement* PMData = NULL;

	PMObj.SMCreate();
	if (PMObj.SMCreateError) {
		Console::WriteLine("ERROR: Process Management SM Object not created");
	}
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("ERROR: Process Management SM Object not accessed");
	}

	PMData = (ProcessManagement*)PMObj.pData;

	//start all 5 modules
	StartProcesses();

	
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
	*/


}