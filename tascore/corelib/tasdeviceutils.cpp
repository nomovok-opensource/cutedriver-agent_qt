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
 


#include "tasdeviceutils.h"


/*!
    \class TasDeviceUtils
    \brief Provides device dependent operations.    
    
    TasNativeUtils contains generic operations for device specific calls. Use this class
    to create operations that implement device specifics. Compare to TasNativeUtils, 
    this class should be used that are not only platform (i.e. Windows/Unix/MACOS), 
    but also device specific (e.g. maemo 5)
*/

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

/*!
  Resets the inactivity timeout for the device. Usually for disabling the screensaver.
*/


void TasDeviceUtils::resetInactivity() 
{
    //NOP
}

/*!
  Returns the heap size of the process. 
  -1 means not supported.
 */
int TasDeviceUtils::currentProcessHeapSize()
{
    return -1;
}

/*!
  Not supported
 */
void TasDeviceUtils::addSystemInformation(TasObject& object)
{}


qreal TasDeviceUtils::currentProcessCpuTime()
{
    return -1;
}

/*!
  Not supported
 */
void TasDeviceUtils::addSystemMemoryStatus(TasObject& object)
{
    object.addAttribute("total",-1);
    object.addAttribute("available",-1);
}



/*!
  True returned so that server is not started in environments that do not implement this method.
 */
bool TasDeviceUtils::isServerRunning()
{
    return true;
}

void TasDeviceUtils::sendMouseEvent(int x, int y, Qt::MouseButton button, QEvent::Type type)
{
}


int TasDeviceUtils::getOrientation()
{
    return -1;
}
