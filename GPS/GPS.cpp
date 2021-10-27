#include "GPS.h"
#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <math.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::IO::Ports;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

struct GPSContents;

int GPS::connect(String^ hostName, int portNumber)
{
	// Create arrays on heap for data transmission
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(224);

	// Config TCP Client
	Client = gcnew TcpClient(hostName, portNumber);
	
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Check stream availability
	Stream = Client->GetStream();
	Console::Write(Stream->DataAvailable);

	return 1;
}
int GPS::setupSharedMemory()
{
	// Create SM Objects
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("GPSSMObject"), sizeof(SM_GPS));

	// Access SMObjects
	ProcessManagementData->SMAccess();
	SensorData->SMAccess();

	// Assign Pointers to SMObject data
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	GPSData = (SM_GPS*)SensorData->pData;
	
	PMData->Shutdown.Flags.GPS = 0;
	return 1;
}
int GPS::getData()
{
	// Create class object for GPS data
	GPSContents GNSS;
	BytePtr = (unsigned char*)&GNSS;
	startByteStream = (unsigned char*)&GNSS;

	// Read data, check it by iterating through
	if (Stream->DataAvailable) {
		Stream->Read(ReadData, 0, ReadData->Length);
		
		Start = checkData();
		for (int i = Start; i < Start + sizeof(GNSS); i++) {
			*(BytePtr++) = ReadData[i];

		}

		// Error checking CRC, GPSContents - 4 is int CRC in the struct
		unsigned long CRCval = CalculateBlockCRC32(sizeof(GPSContents) - 4, startByteStream);
		if (GNSS.CRC == CRCval) {
			
			// If error checking was ok, assign the values to the class object
			gpsEasting = GNSS.Easting;
			gpsNorthing = GNSS.Northing;
			gpsHeight = GNSS.Height;
			
			// Check in the console to see if it's good to go
			Console::WriteLine("Northing: {0,10:F3}  Easting: {1,10:F3}  Height: {2,10:F3}", GNSS.Northing,
				GNSS.Easting, GNSS.Height);
			
			sendDataToSharedMemory();

		}
	}

	return 1;
}

int GPS::checkData()
{
	// From Week 7 Slides - Checking Buffer is correct
	unsigned int Header = 0;
	int i = 0;
	unsigned char Data;

	do
	{
		Data = ReadData[i++];
		Header = ((Header << 8) | Data);
	} while (Header != 0xaa44121c);

	return i - 4;
}

int GPS::sendDataToSharedMemory()
{
	// Assign the received data to the SM
	GPSData->easting = gpsEasting;
	GPSData->northing = gpsNorthing;
	GPSData->height = gpsHeight;

	return 1;
}

bool GPS::getShutdownFlag()
{
	// Get shutdown flag for main - used for heartbeats and shutting down the solution
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.GPS;
}

int GPS::setHeartbeat(bool heartbeat)
{
	double WaitAndSee = 0.00;

	// Toggle the heartbeat between PM and GPS - Constantly flipping to check status
	if (PMData->Heartbeat.Flags.GPS == 0) {
		PMData->Heartbeat.Flags.GPS = 1;

		//Debugging
		//Console::WriteLine("HB GPS: " + PMData->Heartbeat.Flags.GPS);

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

GPS::~GPS()
{
	// Not sure if things need to be deleted but I guess it doesn't hurt
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
