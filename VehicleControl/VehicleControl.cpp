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

	StudID = gcnew String("5259999\n");

	Client = gcnew TcpClient(hostName, portNumber);

	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	Stream = Client->GetStream();

	SendData = System::Text::Encoding::ASCII->GetBytes(StudID);

	Stream->Write(SendData, 0, SendData->Length);

	System::Threading::Thread::Sleep(10);

	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	Console::WriteLine(ResponseData);

	return 1;
}
int VehicleControl::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));

	ProcessManagementData->SMAccess();
	SensorData->SMAccess();
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	VehicleControl = (SM_VehicleControl*)SensorData->pData;
	PMData->Shutdown.Flags.VehicleControl = 0;

	return 1;
}
int VehicleControl::getData()
{
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
	double WaitAndSee = 0.00;

	if (PMData->Heartbeat.Flags.VehicleControl == 0) {
		PMData->Heartbeat.Flags.VehicleControl = 1;

		//Debugging
		//Console::WriteLine("HB VC: " + PMData->Heartbeat.Flags.VehicleControl);

		WaitAndSee = 0.00;
	}
	else {
		WaitAndSee += 25;
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
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}