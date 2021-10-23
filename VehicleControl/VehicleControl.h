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

ref class VehicleControl : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	~VehicleControl();

protected:
	
	int PortNumber;				//Server PortNum
	TcpClient^ Client;
	NetworkStream^ Stream;		//handle for NetworkStream obj
	System::String^ IPAddress;	//Server IP Address
	System::String^ zID;		//User zID (z5259999)

	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData;
	String^ ResponseData;
	String^ VehicleInput;

	int flag = 0;

	ProcessManagement* PMData;	// PM Data Pointer
	SM_VehicleControl* VehicleData;


};
