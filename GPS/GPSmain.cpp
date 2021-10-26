
#include "GPS.h"
#include "SMStructs.h"
#include <SMObject.h>

#include <iostream>
#include <conio.h>

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace Net;
using namespace Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {
	
	GPS GPSModule;
	GPSModule.setupSharedMemory();

	// A
	int PortNumber = 24000;
	String^ hostName = "192.168.1.200";

	GPSModule.connect(hostName, PortNumber);
	while (!GPSModule.getShutdownFlag()) {
		GPSModule.getData();
		GPSModule.checkData();
		GPSModule.sendDataToSharedMemory();
		GPSModule.setHeartbeat(1);
	}

	GPSModule.~GPS();
	return 0;
}