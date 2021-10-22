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
	
	ProcessManagement* PMData;	// PM Data Pointer
	SM_VehicleControl* VehicleData;
};
