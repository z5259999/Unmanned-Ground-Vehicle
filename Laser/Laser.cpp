
// Credited Authors: Jay Katupitiya, James Stevens
// Codebase derived from week 4 Pre-recorded lecture_3 [Accessed from 4 Oct 2021] 
// Available: https://moodle.telt.unsw.edu.au/mod/resource/view.php?id=4164111

#include "Laser.h"
#using <System.dll>
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

int Laser::connect(String^ hostName, int portNumber) {

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
		Console::WriteLine("Connected to Laser");
	}

	// Initialise strings for data
	zID = gcnew String("z5259999\n");
	ScanReq = gcnew String("sRN LMDscandata");

	// Array of chars for client reading/writing
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	// Get the current datastream for reading/writing
	Stream = Client->GetStream();

	// String to unisgned char, used to identify user
	SendData = System::Text::Encoding::ASCII->GetBytes(zID);
	Stream->Write(SendData, 0, SendData->Length);
	
	// Wait, then read incoming datastream, assign to ResponseData
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);

	Console::WriteLine(ResponseData);
	SendData = System::Text::Encoding::ASCII->GetBytes(ScanReq);	

	return 1;

}

int Laser::setupSharedMemory() {

	// Create the SM Objects
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("Laser"), sizeof(SM_Laser));
	
	// Attempt to access PM
	ProcessManagementData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("ERROR: PM SM Object not accessed");
	}
	// Attempt to access Laser
	SensorData->SMAccess();
	if (SensorData->SMAccessError) {
		Console::WriteLine("ERROR: Laser SM Object not accessed");
	}

	// Point to shared memory
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	LaserData = (SM_Laser*)SensorData->pData;
	
	// Set flag to 0 by default, ensure it isn't shut down
	PMData->Shutdown.Flags.Laser = 0;

	return 1;

}

int Laser::getData() {

	// Data transmission: start (0x02) -> stop (0x03)
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	
	// Read incoming datastream
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);

	// Convert unsigned char to string
	System::Text::Encoding::ASCII->GetString(ReadData);

	return 1;
}

int Laser::checkData() {

	ResponseData = ResponseData->Replace(":", "");
	StringFrags = ResponseData->Split(' ');

	Console::WriteLine(StringFrags[1]);

	Range = gcnew array<double>(NumRanges);
	RangeX = gcnew array<double>(NumRanges);
	RangeY = gcnew array<double>(NumRanges);

	StartAngle = System::Convert::ToInt32(StringFrags[23], 16);
	Resolution = System::Convert::ToInt32(StringFrags[24], 16) / 10000.0;
	NumRanges = System::Convert::ToInt32(StringFrags[25], 16);

	// Polar to Cartesian
	for (int i = 0; i < NumRanges; i++)
	{
		Range[i] = System::Convert::ToInt32(StringFrags[26 + i], 16);
		RangeX[i] = Range[i] * Math::Sin(i * Resolution * Math::PI / 180.0);
		RangeY[i] = Range[i] * Math::Cos(i * Resolution * Math::PI / 180.0);

		Console::WriteLine("Resolution: {0, 0:F3}", i * Resolution);

	}

	return 1;

}

int Laser::sendDataToSharedMemory() {

	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		LaserData->x[i] = RangeX[i];
		LaserData->y[i] = RangeY[i];
		Console::WriteLine("x:{0, 0:F4} y:{1, 0:F4}", RangeX[i], RangeY[i]);
	}

	return 1;

}

bool Laser::getShutdownFlag() {

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.Laser;

}

int Laser::setHeartbeat(bool heartbeat) {

	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	
	// Heartbeats: Laser CRITICAL
	// Flip the flag from 0 to 1 - Communicates with ProcessManagement.cpp
	if (PMData->Heartbeat.Flags.Laser == 0) {
		PMData->Heartbeat.Flags.Laser = 1;
		PMData->ModCounters[TIME_LASER] = 0.00;
	}
	// No response for CRITICAL PROCESS - Kill Program
	else {
		PMData->ModCounters[TIME_LASER] += 25;
		if (PMData->ModCounters[TIME_LASER] > TIMEOUT) {
			PMData->Shutdown.Status = 0xFF;
		}
	}

	//Threading time
	Thread::Sleep(25);

	if (PMData->Shutdown.Status) {
		exit(0);
	}

	return 1;

}

Laser::~Laser() {

	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;

}