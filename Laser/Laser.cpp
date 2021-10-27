#using <System.dll>
#include "laser.h"
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


int Laser::connect(String^ hostName, int portNumber)
{
	AskScan = gcnew String("sRN LMDscandata");
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

	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);


	return 1;
}
int Laser::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("LaserSMObject"), sizeof(SM_Laser));

	ProcessManagementData->SMAccess();
	SensorData->SMAccess();
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	LaserData = (SM_Laser*)SensorData->pData;
	PMData->Shutdown.Flags.Laser = 0;

	return 1;
}
int Laser::getData()
{
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);

	System::Threading::Thread::Sleep(120);

	Stream->Read(ReadData, 0, ReadData->Length);

	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);

	return 1;
}
int Laser::checkData()
{
	array<String^>^ LaserData = ResponseData->Split(' ');
	Console::WriteLine(LaserData[1]);

	if (LaserData[1] == "LMDscandata") {

		Console::WriteLine(LaserData[20]);
		double StartAngle = System::Convert::ToInt32(LaserData[23], 16);
		double AngularStep = System::Convert::ToInt32(LaserData[24], 16) / 10000.0;
		double NumberData = System::Convert::ToInt32(LaserData[25], 16);

		Console::WriteLine("The start angle: {0, 0:F0}", StartAngle);
		Console::WriteLine("The step size: {0, 0:F3}", AngularStep);
		Console::WriteLine("The number of data  points: {0, 0:F0}", NumberData);


		XRange = gcnew array<double>(NumberData);
		YRange = gcnew array<double>(NumberData);
		double temp, angle;

		for (int i = 0; i < NumberData; i++) {
			temp = System::Convert::ToInt32(LaserData[26 + i], 16);
			XRange[i] = temp * Math::Sin(i * AngularStep * Math::PI / 180);
			YRange[i] = temp * Math::Cos(i * AngularStep * Math::PI / 180);

		}
	}

	return 1;
}
int Laser::sendDataToSharedMemory()
{
	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		LaserData->x[i] = XRange[i];
		LaserData->y[i] = YRange[i];
		Console::WriteLine("x:{0, 0:F4} y:{1, 0:F4}", XRange[i], YRange[i]);
	}

	return 1;
}
bool Laser::getShutdownFlag()
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.Laser;
}
int Laser::setHeartbeat(bool heartbeat)
{
	double WaitAndSee = 0.00;

	if (PMData->Heartbeat.Flags.Laser == 0) {
		PMData->Heartbeat.Flags.Laser = 1;

		//Debugging
		//Console::WriteLine("HB Laser: " + PMData->Heartbeat.Flags.Laser);

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
Laser::~Laser()
{
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}
