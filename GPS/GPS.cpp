
#include "GPS.h"
#include "SMStructs.h"
#include <SMObject.h>

#include <iostream>
#include <conio.h>

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int GPS::connect(String^ hostName, int portNumber)
{
	// Create Client
	Client = gcnew TcpClient(hostName, portNumber);

	// Configure Client (Client default 'Settings')
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->SendBufferSize = 1024;
	Client->ReceiveBufferSize = 1024;

	// Check Connection Status
	if (Client->Connected)
	{
		Console::WriteLine("Connected to GPS");
	}

	// Array of chars for client reading/writing
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(224);

	// Get the current datastream for reading/writing
	Stream = Client->GetStream();

	return 1;
}
int GPS::setupSharedMemory()
{
	// Create the SM Objects
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("GPS"), sizeof(SM_GPS));

	// Attempt to access PM
	ProcessManagementData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("ERROR: PM SM Object not accessed");
	}
	// Attempt to access GPS
	SensorData->SMAccess();
	if (SensorData->SMAccessError) {
		Console::WriteLine("ERROR: GPS SM Object not accessed");
	}

	// Point to shared memory
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	GPSData = (SM_GPS*)SensorData->pData;

	// Set flag to 0 by default, ensure it isn't shut down
	PMData->Shutdown.Flags.GPS = 0;

	return 1;

}
int GPS::getData()
{
	GPSContents GNSS;

	unsigned char* BytePtr = nullptr;
	BytePtr = (unsigned char*)&GNSS;
	int debugFlag = 1;

	if (Stream->DataAvailable) {
		Stream->Read(ReadData, 0, ReadData->Length);
		Start = checkData();
		for (int i = Start; i < Start + sizeof(GNSS); i++) {
			*(BytePtr++) = ReadData[i];
		}

		east = GNSS.Easting;
		north = GNSS.Northing;
		high = GNSS.Height;

		sendDataToSharedMemory();
		Console::WriteLine("Northing: {0,10:F3}", GPSData->northing);
		Console::WriteLine("Easting: {1,10:F3}", GPSData->easting);
		Console::WriteLine("Height: {2,10:F3}", GPSData->height);

	}

	return 1;
}
int GPS::checkData()
{
	unsigned int Header = 0;
	int i = 0;
	unsigned char Data;
	do
	{
		Data = ReadData[i++];
		Header = ((Header << 8) | Data); //shift header by 8 bits
	} while (Header != 0xaa44121c);
	Start = i - 4;

	return Start;
}
int GPS::sendDataToSharedMemory()
{
	GPSData->easting = east;
	GPSData->northing = north;
	GPSData->height = high;
	return 1;
}
bool GPS::getShutdownFlag()
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.GPS;

}
int GPS::setHeartbeat(bool heartbeat)
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	double WaitTimeG = 0.00;

	if (PMData->Heartbeat.Flags.GPS == 0) {
		PMData->Heartbeat.Flags.GPS == 1;
		WaitTimeG = 0.00;
	}
	else {
		WaitTimeG += 25;
		if (WaitTimeG > TIMEOUT) {
			PMData->Shutdown.Status = 0xFF;
		}
	}

	Thread::Sleep(25);

	if (PMData->Shutdown.Status) {
		exit(0);
	}

	return 1;
}
GPS::~GPS()
{
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}


unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}