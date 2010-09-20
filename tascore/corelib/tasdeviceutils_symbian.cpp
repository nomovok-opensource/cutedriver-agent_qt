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
 


#include <hal.h>
#include <hal_data.h>
#if ( NCP_COMMON_BRANCH_IN_USE == MCL )
#include <qsysteminfo.h>
QTM_USE_NAMESPACE
#endif

#include "tasdeviceutils.h"
#include "taslogger.h"

#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )
#include "gpuinfo_symbian.h"
#endif

_LIT( KQTasServerName, "qttasserver" );

TasDeviceUtils::TasDeviceUtils()
{
    gpuDetailsHandler = 0;
}

GpuMemDetails TasDeviceUtils::gpuMemDetails()
{
#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )
    if(!gpuDetailsHandler){
        gpuDetailsHandler = new GpuMemDetailsHandler();
    }
    return gpuDetailsHandler->gpuData();
#else
    GpuMemDetails details:
    details.isValid = false;
    return details;
#endif
}

void TasDeviceUtils::resetInactivity() 
{
    User::ResetInactivityTime();
}
  
/*!
  Returns the heap size of the process. 
  -1 means not supported.
*/
int TasDeviceUtils::currentProcessHeapSize()
{
    return User::Heap().Size();
}


/*!
  add total mem details
*/
void TasDeviceUtils::addSystemMemoryStatus(TasObject& object)
{
    TInt ram;
    if( HAL::Get(HALData::EMemoryRAM, ram) == KErrNone){
        object.addAttribute("total", ram);
    }
    TInt freeRam;
    if( HAL::Get(HALData::EMemoryRAMFree, freeRam) == KErrNone){
        object.addAttribute("available", freeRam);
    }
}

/*!
  Return cpu time in millis
*/
qreal TasDeviceUtils::currentProcessCpuTime()
{
    TTimeIntervalMicroSeconds cpuTime;
    if(RThread().GetCpuTime(cpuTime) != KErrNone){
        return -1;
    }
    return cpuTime.Int64()/1000.0;
}


/*!
  Add details available from mobility apis
*/
void TasDeviceUtils::addSystemInformation(TasObject& object)
{
    Q_UNUSED(object);
    TasLogger::logger()->debug("TasDeviceUtils::addSystemInformation");
    // #if ( NCP_COMMON_BRANCH_IN_USE == MCL )
    //     TasBaseTraverse traverser;
    //     traverser.resetFilter();
    //     QSystemInfo sysInfo;
    //     TasLogger::logger()->debug("TasDeviceUtils::addSystemInformation systeminfo");
    //     sysInfo.setObjectName("SystemInfo");
    //     traverser.addObjectDetails(&object.addObject(), &sysInfo);
    //     QSystemNetworkInfo network;
    //     TasLogger::logger()->debug("TasDeviceUtils::addSystemInformation netinfo");
    //     network.setObjectName("NetworkInfo");
    //     traverser.addObjectDetails(&object.addObject(), &network);
    //     QSystemDisplayInfo display;
    //     TasLogger::logger()->debug("TasDeviceUtils::addSystemInformation display");
    //     display.setObjectName("DisplayInfo");
    //     traverser.addObjectDetails(&object.addObject(), &display);
    //     QSystemStorageInfo storage;
    //     TasLogger::logger()->debug("TasDeviceUtils::addSystemInformation storage");
    //     storage.setObjectName("StorageInfo");
    //     traverser.addObjectDetails(&object.addObject(), &storage);
    //     QSystemDeviceInfo deviceInfo;
    //     TasLogger::logger()->debug("TasDeviceUtils::addSystemInformation device");
    //     deviceInfo.setObjectName("DeviceInfo");
    //     traverser.addObjectDetails(&object.addObject(), &deviceInfo);
    // #endif
    TasLogger::logger()->debug("TasDeviceUtils::addSystemInformation out");
}

void TasDeviceUtils::sendMouseEvent(int x, int y, Qt::MouseButton button, QEvent::Type type)
{
    Q_UNUSED(x);   
    Q_UNUSED(y);   
    Q_UNUSED(button);   
    Q_UNUSED(type);   
}

/*!
  Start the qttasserver if not running.
 */
bool TasDeviceUtils::isServerRunning()
{
    //1. look for the process
    bool running = false;
    TFindProcess findProcess;
    TFullName processName;
    while ( findProcess.Next( processName ) == KErrNone){        
        if ( ( processName.Find( KQTasServerName ) != KErrNotFound ) ){
            RProcess process;
            TInt err = process.Open( findProcess );
            if( err == KErrNone){                
                //make sure the process alive
                if( process.ExitType() == EExitPending ){
                    running = true;
                }
                process.Close();
                if(running){
                    break;
                }
            }              
        }
    }
    return running;
}

