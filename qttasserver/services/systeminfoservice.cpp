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
 

#include <QCoreApplication>

#include <tasqtdatamodel.h>
#include <taslogger.h>
#include <tascoreutils.h>
#include <version.h>
#include <tasdeviceutils.h>

#include "systeminfoservice.h"

SystemInfoService::SystemInfoService()
{
    mLogger = new InfoLogger();
}

SystemInfoService::~SystemInfoService()
{
    delete mLogger;
}

bool SystemInfoService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        response.setIsError(false);
        response.setData(systemInfo());
        return true;
    }
    else if (model.service() == INFO_SERVICE ){
        mLogger->performLogService(model, response);         
        return true;
    }
    else{
        return false;
    }
}

QByteArray SystemInfoService::systemInfo()
{
    TasDataModel* model = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = model->addNewObjectContainer(1, qtVersion, "qt");
    TasObject& sutInfo = container.addNewObject(QString::number(QCoreApplication::applicationPid()), "sutInfo", "sutInfo");

    TasObject& sysInfo = sutInfo.addNewObject("1","SystemInformation","SystemInformation");
    TasDeviceUtils::addSystemInformation(sysInfo);

    TasObject& memInfo = sutInfo.addNewObject("1","MemoryStatus","MemoryStatus");
    TasDeviceUtils::addSystemMemoryStatus(memInfo);
    memInfo.addAttribute("qttasMemUsage", TasDeviceUtils::currentProcessHeapSize());
    
    QByteArray xml;
    model->serializeModel(xml);
    delete model;
    return xml;
}
