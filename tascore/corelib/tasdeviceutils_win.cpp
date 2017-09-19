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

#include "taslogger.h"
#include "tasdeviceutils.h"

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <stdio.h>
#include <psapi.h>



TasDeviceUtils::TasDeviceUtils()
{
    gpuDetailsHandler = 0;
    pwrDetailsHandler = 0;
}

GpuMemDetails TasDeviceUtils::gpuMemDetails()
{
    GpuMemDetails details;
    details.isValid = false;
    return details;
}

PwrDetails TasDeviceUtils::pwrDetails()
{
    PwrDetails details;
    details.isValid = false;
    return details;
}
void TasDeviceUtils::stopPwrData()
{}


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




void TasDeviceUtils::sendMouseEvent(int x, int y, Qt::MouseButton button, QEvent::Type type, uint pointerNumber)
{
    TasLogger::logger()->debug("TasDeviceUtils::sendMouseEvent");

    MOUSEINPUT mi = {0};
    mi.dx = x;
    mi.dy = y;
    mi.mouseData = 0;
    mi.time = 0; //system should provide this
    mi.dwExtraInfo = 0;

    if(type == QEvent::MouseButtonPress || type == QEvent::GraphicsSceneMousePress){
        TasLogger::logger()->debug("TasDeviceUtils::sendMouseEvent send event type press");
        if(button == Qt::LeftButton){
            mi.dwFlags = MOUSEEVENTF_LEFTDOWN; //0x0002;
        }
        else if(button == Qt::RightButton){
            mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; //0x0008;
        }
        else if(button == Qt::MidButton){
            mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN; //0x0020;
        }
    }
    else if(type == QEvent::MouseButtonRelease || type == QEvent::GraphicsSceneMouseRelease){
        TasLogger::logger()->debug("TasDeviceUtils::sendMouseEvent send event type release");
        if(button == Qt::LeftButton){
            mi.dwFlags = MOUSEEVENTF_LEFTUP; //0x0004;
        }
        else if(button == Qt::RightButton){
            mi.dwFlags = MOUSEEVENTF_RIGHTUP; //0x0010;
        }
        else if(button == Qt::MidButton){
            mi.dwFlags = MOUSEEVENTF_MIDDLEUP;//0x0040;
        }
    }
    else if(type == QEvent::MouseMove || type == QEvent::GraphicsSceneMouseMove){
        TasLogger::logger()->debug("TasDeviceUtils::sendMouseEvent send event type move");
        double width = GetSystemMetrics(0);
        double height = GetSystemMetrics(1);
        mi.dx = (int)(x * (65535.0 / width));
        mi.dy = (int)(y * (65535.0 / height));
        mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    }
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi = mi;
    TasLogger::logger()->debug("TasDeviceUtils::sendMouseEvent send event type " + QString::number(mi.dwFlags));

    SendInput(1, &input, sizeof(INPUT));
}


/*!
  Not implemented, true returned to avoid autostart.
 */
bool TasDeviceUtils::isServerRunning()
{
    return true;
}


int TasDeviceUtils::getOrientation()
{
    return -1;
}
