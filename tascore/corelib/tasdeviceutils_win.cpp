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
 


#include <windows.h>
#include <stdio.h>
#include <psapi.h>

#include "tasdeviceutils.h"

TasDeviceUtils::TasDeviceUtils()
{
    gpuDetailsHandler = 0;
}

GpuMemDetails TasDeviceUtils::gpuMemDetails()
{
    GpuMemDetails details;
    details.isValid = false;
    return details;
}

void TasDeviceUtils::resetInactivity() 
{
    //not supported
}
  
/*!
  Returns the heap size of the process. 
  -1 that memory information could not be obtained.
 */
int TasDeviceUtils::currentProcessHeapSize()
{
 
    PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(GetCurrentProcess(),&pmc, sizeof(pmc))){
        return pmc.WorkingSetSize;
    }
    else{
        return -1;
    }
}

/*!
  Not supported
 */
void TasDeviceUtils::addSystemMemoryStatus(TasObject& object)
{
     MEMORYSTATUS statex;
     statex.dwLength = sizeof (statex);    
     GlobalMemoryStatus (&statex);
     object.addAttribute("total",QString::number(statex.dwTotalPhys));
     object.addAttribute("available",QString::number(statex.dwAvailPhys));
}

qreal TasDeviceUtils::currentProcessCpuTime()
{
    FILETIME userTime;
    FILETIME sysTime;
    FILETIME createTime;
    FILETIME exitTime;

    if(GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &sysTime, &userTime)){
        ULARGE_INTEGER uliS;
        uliS.LowPart = sysTime.dwLowDateTime; 
        uliS.HighPart = sysTime.dwHighDateTime;        

        ULARGE_INTEGER uliU;
        uliU.LowPart = userTime.dwLowDateTime; 
        uliU.HighPart = userTime.dwHighDateTime;
        return (uliS.QuadPart/10000.0) + (uliU.QuadPart/10000.0);
    }
    else{
        return -1;
    }
}


/*!
  Not supported
 */
void TasDeviceUtils::addSystemInformation(TasObject& object)
{
    //some details for windows...
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo); 
    object.addAttribute("numberOfProcessors", QString::number(siSysInfo.dwNumberOfProcessors));
    object.addAttribute("processorType", QString::number(siSysInfo.dwProcessorType));
    object.addAttribute("processorArchitecture", siSysInfo.wProcessorArchitecture);
}




void TasDeviceUtils::sendMouseEvent(int x, int y, Qt::MouseButton button, QEvent::Type type)
{
}

/*!
  Not implemented, true returned to avoid autostart.
 */
bool TasDeviceUtils::isServerRunning()
{
    return true;
}
