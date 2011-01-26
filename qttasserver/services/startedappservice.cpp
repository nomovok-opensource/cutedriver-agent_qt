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
 
#include <tasqtdatamodel.h>
#include <taslogger.h>

#include "startedappservice.h"


StartedAppsService::StartedAppsService()
{}

StartedAppsService::~StartedAppsService()
{}

bool StartedAppsService::executeService(TasCommandModel& model, TasResponse& response)
{    
    if(model.service() == serviceName()){
        TasCommand* command = getCommandParameters(model, "startedApps");
        if(command){
            startedApplications(*command, response);
        }
        else {
            response.setErrorMessage(QString("Could not parse the startedApps command from the request."));
        }
        return true;
    }
    else {
        return false;
    }

}

/*!
  List started applications. List process names and timestamp for each application.
 */
void StartedAppsService::startedApplications(TasCommand& command, TasResponse& response)
{
    Q_UNUSED(command)
    TasLogger::logger()->debug("StartedAppsService::startedApplications ");

    TasDataModel* model = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = model->addNewObjectContainer(1, qtVersion, "qt");
    TasObject* apps = &container.addNewObject(0, "QApplications", "applications");

    TasClientManager::instance()->startedApplicationsList(*apps);

    QByteArray xml;
    model->serializeModel(xml);    
    delete model;
    response.setData(xml);
}

