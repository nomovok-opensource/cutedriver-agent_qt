/*************************************************************************** 
** 
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
** All rights reserved. 
** Contact: Nokia Corporation (testabilitydriver@nokia.com) 
** 
** This file is part of Testability Driver Qt Agent
** 
** If you have questions regarding the use of this file, please contact 
** Nokia at testabilitydriver@nokia.com . 
** 
** This library is free software; you can redistribute it and/or 
** modify it under the terms of the GNU Lesser General Public 
** License version 2.1 as published by the Free Software Foundation 
** and appearing in the file LICENSE.LGPL included in the packaging 
** of this file. 
** 
****************************************************************************/ 
 


#ifndef TASDEVICEUTILS_H
#define TASDEVICEUTILS_H

#include "tasconstants.h"
#include "tasqtdatamodel.h"

#include <QEvent>
#include <QProcess>


struct GpuMemDetails
{
  bool isValid;  
  qint64 totalMem;
  qint64 usedMem;
  qint64 freeMem;
  qint64 processPrivateMem;
  qint64 processSharedMem;
};

class GpuMemDetailsInterface
{
public:
    virtual ~GpuMemDetailsInterface(){}
    virtual GpuMemDetails gpuData() = 0;
};

struct PwrDetails
{
  int voltage;
  int current;
  bool isValid;
};

class PwrDetailsInterface
{
public:
    virtual ~PwrDetailsInterface(){}
    virtual PwrDetails pwrData() = 0;
};

class TasDeviceUtils 
{
public:
    TasDeviceUtils();
	~TasDeviceUtils(){if(gpuDetailsHandler) delete gpuDetailsHandler;if(pwrDetailsHandler) delete pwrDetailsHandler;}

    static void resetInactivity();
	static int currentProcessHeapSize();
	static void addSystemMemoryStatus(TasObject& object);
	static void addSystemInformation(TasObject& object);
	static void sendMouseEvent(int x, int y, Qt::MouseButton button, QEvent::Type, uint pointerNumber);
	/*!
	  Return cpu time in millis
	*/
	static qreal currentProcessCpuTime();
	
	GpuMemDetails gpuMemDetails();
	PwrDetails    pwrDetails();
	void          stopPwrData();

	static bool isServerRunning();

    static int getOrientation();

#ifdef Q_OS_SYMBIAN
    static bool flipOrigo;
#endif

private:
	GpuMemDetailsInterface* gpuDetailsHandler;
	PwrDetailsInterface* pwrDetailsHandler;

};

#endif
