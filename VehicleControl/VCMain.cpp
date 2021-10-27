#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include "VehicleControl.h" //include from the same directory you are in

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	VehicleControl VehicleClass;

	//Sleep(100);
	VehicleClass.setupSharedMemory();

	int PortNumber = 25000;
	String^ HostName = "192.168.1.200";
	VehicleClass.connect(HostName, PortNumber);

	while (!VehicleClass.getShutdownFlag()) {

		VehicleClass.getData();
		VehicleClass.sendDataToSharedMemory();
		VehicleClass.setHeartbeat(1);
	}

	VehicleClass.~VehicleControl();


	return 0;
}