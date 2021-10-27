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

	System::Threading::Thread::Sleep(100);

	Stream->Read(ReadData, 0, ReadData->Length);

	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);

	return 1;
}
int Laser::checkData()
{
	DataPoints = ResponseData->Split(' ');

	// Checking the start of the data is LMDscandata - Correct set of data
	if (DataPoints[1] == "LMDscandata") {

		Console::WriteLine(DataPoints[20]);
		StartAngle = System::Convert::ToInt32(DataPoints[23], 16);
		Resolution = System::Convert::ToInt32(DataPoints[24], 16) / 10000.0;
		NumberRange = System::Convert::ToInt32(DataPoints[25], 16);

		Console::WriteLine("Start Angle: {0, 0:F}", StartAngle);
		Console::WriteLine("Res: {0, 0:F3}", Resolution);
		Console::WriteLine("Data Points: {0, 0:F}", NumberRange);

		Range = gcnew array<double>(NumberRange);
		RangeX = gcnew array<double>(NumberRange);
		RangeY = gcnew array<double>(NumberRange);

		for (int i = 0; i < NumberRange; i++) {
			Range[i] = System::Convert::ToInt32(DataPoints[26 + i], 16);
			RangeX[i] = Range[i] * Math::Sin(i * Resolution * Math::PI / 180);
			RangeY[i] = -Range[i] * Math::Cos(i * Resolution * Math::PI / 180);

		}
	}

	return 1;
}
int Laser::sendDataToSharedMemory()
{
	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		LaserData->x[i] = RangeX[i];
		LaserData->y[i] = RangeY[i];
		Console::WriteLine("x:{0, 0:F}", RangeX[i]);
		Console::WriteLine("y:{0, 0:F}", RangeY[i]);
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
