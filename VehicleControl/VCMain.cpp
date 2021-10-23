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
	
	VehicleControl VCModule;
	VCModule.setupSharedMemory();
	int PortNumber = 25000;
	String^ hostName = "192.168.1.200";

	VCModule.connect(hostName, PortNumber);
	while (!VCModule.getShutdownFlag()) {
		VCModule.getData();
		VCModule.sendDataToSharedMemory();
		VCModule.setHeartbeat(1);
	}

	VCModule.~VehicleControl();

	return 0;
}