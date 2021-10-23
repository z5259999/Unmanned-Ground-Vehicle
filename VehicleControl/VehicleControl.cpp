#include "VehicleControl.h"
#include "SMStructs.h"
#include "SMObject.h"

#include <math.h>

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
	
	// Create Client
	Client = gcnew TcpClient(hostName, portNumber);

	// Configure Client (Client defauly 'Settings')
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->SendBufferSize = 1024;
	Client->ReceiveBufferSize = 1024;

	// Check Connection Status
	if (Client->Connected)
	{
		Console::WriteLine("Connected to Vehicle Control");
	}
	
	// Array of chars for client reading/writing
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	// Get the current datastream for reading/writing
	Stream = Client->GetStream();

	SendData = System::Text::Encoding::ASCII->GetBytes(zID);
	Stream->Write(SendData, 0, SendData->Length);
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	Console::WriteLine(ResponseData);

	return 1;
}
int VehicleControl::setupSharedMemory()
{
	// Create the SM Objects
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("VehicleControl"), sizeof(SM_VehicleControl));

	// Attempt to access PM
	ProcessManagementData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("ERROR: PM SM Object not accessed");
	}
	// Attempt to access VC
	SensorData->SMAccess();
	if (SensorData->SMAccessError) {
		Console::WriteLine("ERROR: VC SM Object not accessed");
	}

	// Point to shared memory
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	VehicleData = (SM_VehicleControl*)SensorData->pData;

	// Set flag to 0 by default, ensure it isn't shut down
	PMData->Shutdown.Flags.VehicleControl = 0;

	return 1;

}
int VehicleControl::getData()
{
	VehicleInput = gcnew String("# " + VehicleData->Steering.ToString("F3") + " " + VehicleData->Speed.ToString("F3") + " " + flag + " #");
	flag = !flag;
	Console::WriteLine(VehicleInput);

	return 1;
}
int VehicleControl::checkData()
{
	return 1;
}
int VehicleControl::sendDataToSharedMemory()
{
	SendData = System::Text::Encoding::ASCII->GetBytes(VehicleInput);
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	System::Threading::Thread::Sleep(30);
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
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}
