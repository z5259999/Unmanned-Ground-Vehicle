#pragma once
#using <System.dll>
#include <SMObject.h>
#include <SMStructs.h>
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <UGV_module.h>
#include "smstructs.h"


using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

ref class Laser : public UGV_module
{
public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	~Laser();

protected:

	int PortNumber;				//Server PortNum
	TcpClient^ Client;
	NetworkStream^ Stream;		//handle for NetworkStream obj
	System::String^ IPAddress;	//Server IP Address
	System::String^ zID;		//User zID (z5259999)
	System::String^ ScanReq;	//

	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;

	array<String^>^ StringFrags;
	String^ ResponseData;

	// Laser Scanning data
	double StartAngle;			//Initialise scanning angle
	double Resolution;			//Step of angular motion
	double NumRanges;
	array<double>^ Range;
	array<double>^ RangeX;
	array<double>^ RangeY;

	// Shared Memory
	ProcessManagement* PMData;	// PM Data Pointer
	SM_Laser* LaserData; // Laser Data Pointer

};
