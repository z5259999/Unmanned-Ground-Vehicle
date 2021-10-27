#using <System.dll>
#include "VehicleControl.h"
#include <Windows.h>
#include <conio.h>
#include <math.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


int VehicleControl::connect(String^ hostName, int portNumber)
{
	
	//Create a TCP Client using zID Authorisation
	StudID = gcnew String("5259999\n");
	Client = gcnew TcpClient(hostName, portNumber);

	// Config TCP Client
	Client->NoDelay = true; 
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Create arrays on heap for data transmission
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	Stream = Client->GetStream();

	// Write data to stream
	SendData = System::Text::Encoding::ASCII->GetBytes(StudID);
	Stream->Write(SendData, 0, SendData->Length);
	System::Threading::Thread::Sleep(25);

	// Read data from stream
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	Console::WriteLine(ResponseData);

	return 1;
}
int VehicleControl::setupSharedMemory()
{
	// Create SM Objects
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));

	// Access SMObjects
	ProcessManagementData->SMAccess();
	SensorData->SMAccess();

	// Assign Pointers to SMObject data
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	VehicleControl = (SM_VehicleControl*)SensorData->pData;
	PMData->Shutdown.Flags.VehicleControl = 0;

	return 1;
}
int VehicleControl::getData()
{
	// Writes the necessary datastream from the spec
	VehicleInput = gcnew String("# " + VehicleControl->Steering.ToString("F3") + " " + VehicleControl->Speed.ToString("F3") + " " + flag + " #");
	flag = !flag;
	Console::WriteLine(VehicleInput);

	return 1;
}
int VehicleControl::checkData()
{
	// Nothing to Check!
	return 1;
}
int VehicleControl::sendDataToSharedMemory()
{
	// Transmit data
	SendData = System::Text::Encoding::ASCII->GetBytes(VehicleInput);
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	// Wait before moving on
	System::Threading::Thread::Sleep(25);
	return 1;
}
bool VehicleControl::getShutdownFlag()
{
	// Get shutdown flag for main - used for heartbeats and shutting down the solution
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.VehicleControl;
}
int VehicleControl::setHeartbeat(bool heartbeat)
{
	double WaitAndSee = 0.00;

	// Toggle the heartbeat between PM and VC - Constantly flipping to check status
	if (PMData->Heartbeat.Flags.VehicleControl == 0) {
		PMData->Heartbeat.Flags.VehicleControl = 1;

		//Debugging
		//Console::WriteLine("HB VC: " + PMData->Heartbeat.Flags.VehicleControl);

		WaitAndSee = 0.00;
	}
	else {
		WaitAndSee += 25;
		// CRITICAL OPERATION: Shutdown entire program if VC isn't working
		if (WaitAndSee > TIMEOUT) {
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
	// Not sure if things need to be deleted but I guess it doesn't hurt
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}