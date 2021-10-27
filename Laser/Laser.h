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

	array<String^>^ DataPoints;

	double StartAngle;
	double Resolution;
	double NumberRange;

	SMObject* PMObj;
	SMObject* LaserSMObject;

	array<double>^ Range;
	array<double>^ RangeX;
	array<double>^ RangeY;



};


