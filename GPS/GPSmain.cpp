
#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include "GPS.h" 

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	GPS GPSClass;

	GPSClass.setupSharedMemory();

	int PortNumber = 24000;
	String^ HostName = "192.168.1.200";
	GPSClass.connect(HostName, PortNumber);

	while (!GPSClass.getShutdownFlag()) {

		GPSClass.getData();
		GPSClass.setHeartbeat(1);

	}

	GPSClass.~GPS();

	return 0;
}