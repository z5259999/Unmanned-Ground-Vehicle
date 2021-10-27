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

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(224);

	Client = gcnew TcpClient(hostName, portNumber);
	
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;


	Stream = Client->GetStream();
	Console::Write(Stream->DataAvailable);

	return 1;
}
int GPS::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("GPSSMObject"), sizeof(SM_GPS));

	ProcessManagementData->SMAccess();
	SensorData->SMAccess();
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	GPSData = (SM_GPS*)SensorData->pData;
	
	PMData->Shutdown.Flags.GPS = 0;
	return 1;
}
int GPS::getData()
{
	GPSContents GNSS;
	BytePtr = (unsigned char*)&GNSS;
	startByteStream = (unsigned char*)&GNSS;

	if (Stream->DataAvailable) {
		Stream->Read(ReadData, 0, ReadData->Length);
		//
		Start = checkData();
		for (int i = Start; i < Start + sizeof(GNSS); i++) {
			*(BytePtr++) = ReadData[i];

		}

		unsigned long temp = CalculateBlockCRC32(sizeof(GPSContents) - 4, startByteStream);
		if (GNSS.CRC == temp) {
			
			gpsEasting = GNSS.Easting;
			gpsNorthing = GNSS.Northing;
			gpsHeight = GNSS.Height;
			
			Console::WriteLine("Northing: {0,10:F3}  Easting: {1,10:F3}  Height: {2,10:F3}", GNSS.Northing,
				GNSS.Easting, GNSS.Height);
			
			sendDataToSharedMemory();

		}
	}

	return 1;
}

int GPS::checkData()
{
	// From Week 7 Slides
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
	GPSData->easting = gpsEasting;
	GPSData->northing = gpsNorthing;
	GPSData->height = gpsHeight;

	return 1;
}

bool GPS::getShutdownFlag()
{
	ProcessManagement* PMData = (ProcessManagement*)ProcessManagementData->pData;
	return PMData->Shutdown.Flags.GPS;
}

int GPS::setHeartbeat(bool heartbeat)
{
	double WaitAndSee = 0.00;

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
