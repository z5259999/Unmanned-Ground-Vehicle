#include "Laser.h"
//#include <math.h>

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int Laser::connect(String^ hostName, int portNumber) {

	//Create Client
	Client = gcnew TcpClient(hostName, portNumber);
	
	//Configure Client
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->SendBufferSize = 1024;
	Client->ReceiveBufferSize = 2048;

	if (Client->Connected)
	{
		Console::WriteLine("Connected to Laser");
	}

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2048);

	Stream = Client->GetStream();

	zID = gcnew String("z5259999\n");
	ScanReq = gcnew String("sRN LMDscandata");

	SendData = System::Text::Encoding::ASCII->GetBytes(zID);
	Stream->Write(SendData, 0, SendData->Length);
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);

	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	Console::WriteLine(ResponseData);
	SendData = System::Text::Encoding::ASCII->GetBytes(ScanReq);	

	return 1;

}

int Laser::setupSharedMemory() {

	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	//SensorData = new SMObject(_TEXT("Laser"), sizeof(SM_Laser));
	
	ProcessManagementData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("ERROR: PM SM Object not accessed");
	}

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;

	return 1;

}

int Laser::getData() {

	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);

	System::Threading::Thread::Sleep(10);

	Stream->Read(ReadData, 0, ReadData->Length);
	System::Text::Encoding::ASCII->GetString(ReadData);
	ResponseData = ResponseData->Replace(":", "");

	StringFrags = ResponseData->Split(' ');

	StartAngle = System::Convert::ToInt32(StringFrags[23], 16);
	Resolution = System::Convert::ToInt32(StringFrags[24], 16) / 10000.0;
	NumRanges = System::Convert::ToInt32(StringFrags[25], 16);

	Range = gcnew array<double>(NumRanges);
	RangeX = gcnew array<double>(NumRanges);
	RangeY = gcnew array<double>(NumRanges);

	for (int i = 0; i < NumRanges; i++)
	{
		Range[i] = System::Convert::ToInt32(StringFrags[26 + i], 16);
		RangeX[i] = Range[i] * Math::Sin(i * Resolution * Math::PI / 180.0);
		RangeY[i] = Range[i] * Math::Cos(i * Resolution * Math::PI / 180.0);

		Console::WriteLine("Angle: {0:F}", i * Resolution);

	}

	//Laser::~Laser();

	return 1;

}

int Laser::checkData() {


	return 1;

}

int Laser::sendDataToSharedMemory() {

	return 1;

}

bool Laser::getShutdownFlag() {

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.Laser;


}

int Laser::setHeartbeat(bool heartbeat) {

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	double WaitTimeL = 0.00;

	if (PMData->Heartbeat.Flags.Laser == 0) {
		PMData->Heartbeat.Flags.Laser = 1;
		WaitTimeL = 0.00;
	}
	else {
		WaitTimeL += 25;
		if (WaitTimeL > TIMEOUT) {
			PMData->Shutdown.Status = 0xFF;
		}
	}

	Thread::Sleep(25);

	if (PMData->Shutdown.Status) {
		exit(0);
	}

	return 1;

}

Laser::~Laser() {

	delete ProcessManagementData;

}