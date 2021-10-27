#pragma once
#include <UGV_module.h>
#include <SMObject.h>
#include <smstructs.h>
#using <System.dll>
#include <Windows.h>


using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
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
	
	ProcessManagement* PMData;
	SM_Laser* LaserData;
	int PortNumber;

	array<unsigned char>^ SendData;

	String^ AskScan;
	String^ StudID;
	String^ ResponseData;
	double TimeStamp;
	__int64 Frequency;
	__int64 Counter;
	int Shutdown;
	
	// ADD CLIENT AND RECV DATA

	double StartAngle;
	double AngularStep;
	double NumberData;
	bool temp;
	SMObject* PMObj;
	SMObject* LaserSMObject;
	array<double>^ XRange;
	array<double>^ YRange;



};


