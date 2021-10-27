
#include <UGV_module.h>
#include <smstructs.h>
#using <System.dll>

using namespace System;
using namespace System::IO::Ports;


#define CRC32_POLYNOMIAL 0xEDB88320L

#pragma pack(4)

struct GPSDataStruct {
	unsigned int Header = 0xaa44121c;
	unsigned char DiscardLot1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char DiscardLot2[40];
	unsigned int CRC;
};
unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer);



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
	ProcessManagement* PMData;
	SM_GPS* GPSData;
	double TimeStamp;
	__int64 Frequency;
	__int64 Counter;
	int Shutdown;
	unsigned char* startBytePtr;
	unsigned char* BytePtr = nullptr;
	int Start;
	double tempNorthing;
	double tempEasting;
	double tempHeight;
	SerialPort^ Port = nullptr;
	String^ PortName = nullptr;
	array<unsigned char>^ SendData;

	//NetworkStream^ Stream;


	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)

};

