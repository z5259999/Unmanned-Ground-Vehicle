#pragma once
#include <UGV_module.h>
#include <smstructs.h>
#using <System.dll>

#define CRC32_POLYNOMIAL 0xEDB88320L

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

struct GPSContents {
	unsigned int Header;
	unsigned char Discard1[40];
	unsigned char Discard2[40];
	double Northing;
	double Easting;
	double Height;
	unsigned int Checksum;
};

ref class GPS : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	~GPS();

protected:
	
	int PortNumber;				//Server PortNum
	TcpClient^ Client;
	NetworkStream^ Stream;		//handle for NetworkStream obj
	String^ PortName;
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;
	unsigned int Checksum;
	int Start;
	double east;
	double north;
	double high;

	ProcessManagement* PMData;	// PM Data Pointer
	SM_GPS* GPSData; // Laser Data Pointer
};

