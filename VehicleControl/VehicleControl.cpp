#include "VehicleControl.h"
#include "SMStructs.h"
#include "SMObject.h"

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

int VehicleControl::connect(String^ hostName, int portNumber)
{
	// YOUR CODE HERE
	return 1;
}
int VehicleControl::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	//SensorData = new SMObject(_TEXT("VehicleControl"), sizeof(SM_VehicleControl));

	ProcessManagementData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("ERROR: PM SM Object not accessed");
	}

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;

	return 1;

}
int VehicleControl::getData()
{
	// YOUR CODE HERE
	return 1;
}
int VehicleControl::checkData()
{
	// YOUR CODE HERE
	return 1;
}
int VehicleControl::sendDataToSharedMemory()
{
	// YOUR CODE HERE
	return 1;
}
bool VehicleControl::getShutdownFlag()
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.VehicleControl;

}
int VehicleControl::setHeartbeat(bool heartbeat)
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	double WaitTimeV = 0.00;

	if (PMData->Heartbeat.Flags.VehicleControl == 0) {
		PMData->Heartbeat.Flags.VehicleControl == 1;
		WaitTimeV = 0.00;
	}
	else {
		WaitTimeV += 25;
		if (WaitTimeV > TIMEOUT) {
			PMData->Shutdown.Status = 0xFF;
		}
	}

	Thread::Sleep(25);

	if (PMData->Shutdown.Status) {
		exit(0);
	}

	return 1;
}
VehicleControl::~VehicleControl()
{
	// YOUR CODE HERE
	delete ProcessManagementData;
}
