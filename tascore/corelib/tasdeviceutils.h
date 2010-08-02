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
#include "tasbasetraverse.h"

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

class TAS_EXPORT TasDeviceUtils 
{
public:
    TasDeviceUtils();
	~TasDeviceUtils(){if(gpuDetailsHandler) delete gpuDetailsHandler;}

    static void resetInactivity();
	static int currentProcessHeapSize();
	static void addSystemMemoryStatus(TasObject& object);
	static void addSystemInformation(TasObject& object);
	static void sendMouseEvent(int x, int y, Qt::MouseButton button, QEvent::Type);
	/*!
	  Return cpu time in millis
	*/
	static qreal currentProcessCpuTime();
	
	GpuMemDetails gpuMemDetails();

	static bool isServerRunning();

private:
	GpuMemDetailsInterface* gpuDetailsHandler;

};

#endif
