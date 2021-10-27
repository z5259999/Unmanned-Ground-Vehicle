#pragma once

#ifndef SMSTRUCTS_H
#define SMSTRUCTS_H

#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


#define STANDARD_LASER_LENGTH 361


struct SM_Laser
{
	double x[STANDARD_LASER_LENGTH];
	double y[STANDARD_LASER_LENGTH];
};

struct SM_VehicleControl
{
	double Speed;
	double Steering;
};

struct SM_GPS
{
	double northing;
	double easting;
	double height;
};

struct UnitFlags
{
	unsigned char	ProcessManagement : 1,	//NONCRITICAL
		Laser : 1,				//NONCRITICAL
		Display : 1,			//NONCRITICAL
		VehicleControl : 1,		//NONCRITICAL
		GPS : 1,				//NONCRITICAL
		Camera : 1,				//NONCRITICAL
		Garbage : 3;
};

union ExecFlags
{
	UnitFlags Flags;
	unsigned short Status;
};

struct ProcessManagement
{
	ExecFlags Heartbeat;
	ExecFlags Shutdown;
	ExecFlags PMHeartbeat;
	double ModCounter[5];
	long int LifeCounter;
};

#define NONCRITICALMASK 0xff	//0 011 0000
#define CRITICALMASK 0x0		//0 100 1111

#define WAIT_LASER 4
#define WAIT_VEHICLE 3
#define WAIT_DISPLAY 2
#define WAIT_GPS 1
#define WAIT_CAMERA 0

#define TIMEOUT 50

#endif
