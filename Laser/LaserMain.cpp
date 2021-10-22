// Credited Authors: Jay Katupitiya, James Stevens
// Codebase derived from week 4 Pre-recorded lecture_3 [Accessed from 4 Oct 2021] 
// Available: https://moodle.telt.unsw.edu.au/mod/resource/view.php?id=4164111


#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "SMStructs.h"
#include "SMObject.h"
#include "Laser.h"

using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System;
using namespace System::Net::Sockets;
using namespace Net;
using namespace Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {
	

	Laser LaserModule;
	LaserModule.setupSharedMemory();

	int PortNumber = 23000;
	String^ hostName = "192.168.1.200";

	LaserModule.connect(hostName, PortNumber);
	while (!LaserModule.getShutdownFlag()) {
		LaserModule.getData();
		LaserModule.checkData();
		LaserModule.sendDataToSharedMemory();
		LaserModule.setHeartbeat(1);
	}

	LaserModule.~Laser();
	return 0;
}