
#include <UGV_module.h>
#include <smstructs.h>
#using <System.dll>

using namespace System;
using namespace System::IO::Ports;


#define CRC32_POLYNOMIAL 0xEDB88320L

#pragma pack(4)

struct GPSContents {
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
	
	unsigned char* startByteStream;
	unsigned char* BytePtr = nullptr;
	int Start;

	double gpsNorthing;
	double gpsEasting;
	double gpsHeight;

	String^ PortName = nullptr;
	array<unsigned char>^ SendData;

	ProcessManagement* PMData;
	SM_GPS* GPSData;

};

