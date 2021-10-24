
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

int Laser::connect(String^ hostName, int portNumber)
{
	Client = gcnew TcpClient(hostName, portNumber);
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	Stream = Client->GetStream();
	zID = gcnew String("5259999\n");
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

int Laser::getData()
{


	// YOUR CODE HERE
	return 1;
}
int Laser::checkData()
{
	Stream->WriteByte(0x02);
	// Write command asking for data
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	//Remove ":" from the string
	ResponseData = ResponseData->Replace(":", "");

	StringArray = ResponseData->Split(' ');
	StartAngle = System::Convert::ToInt32(StringArray[23], 16);
	Resolution = System::Convert::ToInt32(StringArray[24], 16) / 10000.0;
	NumRanges = System::Convert::ToInt32(StringArray[25], 16);

	Range = gcnew array<double>(NumRanges);
	RangeX = gcnew array<double>(NumRanges);
	RangeY = gcnew array<double>(NumRanges);
	for (int i = 0; i < NumRanges; i++)
	{
		Range[i] = System::Convert::ToInt32(StringArray[26 + i], 16);
		RangeX[i] = Range[i] * Math::Sin(i * Resolution * Math::PI / 180.0);
		RangeY[i] = -Range[i] * Math::Cos(i * Resolution * Math::PI / 180.0);

		// Print the received string on the screen
		Console::WriteLine("The angle is: {0:F}", i * Resolution);
		Console::WriteLine("The X coordinate is: {0:F}", RangeX[i]);
		Console::WriteLine("The Y coordinate is: {0:F}", RangeY[i]);
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