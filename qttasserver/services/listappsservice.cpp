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
 

#include <QProcess>
#include <tasqtdatamodel.h>
#include <taslogger.h>
#include <tasserver.h>
#include <QDateTime>

#include "listappsservice.h"
#include "tasnativeutils.h"

ListAppsService::ListAppsService()
{
    mStartTime = QDateTime::currentMSecsSinceEpoch();
}

ListAppsService::~ListAppsService()
{}

bool ListAppsService::executeService(TasCommandModel& model, TasResponse& response)
{    
    bool consumed = false;
    if(model.service() == serviceName()){
        TasCommand* command = getCommandParameters(model, "listApps");
        if(command){
            listApplications(*command, response);
        }
        else {
            response.setErrorMessage(QString("Could not parse the listApps command from the request."));
        }
        consumed = true;
    }
    if(model.service() == "listRunningProcesses"){
        listRunninProcesses(response);   
        consumed = true;
    }
    return consumed;
}

/*!
  List running applications. List process names and process id for each application.
 */
void ListAppsService::listApplications(TasCommand& command, TasResponse& response)
{
    Q_UNUSED(command)
    TasLogger::logger()->debug("ListAppsService::listApplications ");

    TasDataModel* model = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = model->addNewObjectContainer(1, qtVersion, "qt");
    TasObject& apps = container.addNewObject("0", "QApplications", "applications");

    TasClientManager::instance()->applicationList(apps);

    // Add HostAddress and HostPort on the Apps List message too.
    TasServer* tasServer = (TasServer*) response.requester()->parent()->parent();
    QString address = tasServer->getServerAddress();
    QString port = QString::number(tasServer->getServerPort());
    TasObject& hostAddress = container.addNewObject("1", "QHostAddress", "hostAddresses");
    hostAddress.addNewObject(0, address, "HostAddress");
    hostAddress.addNewObject(0, port, "HostPort");

    //add uptime also
    QDateTime started = QDateTime::fromMSecsSinceEpoch(mStartTime);
    QDateTime current = QDateTime::currentDateTime();
    int days = current.daysTo(current);
    if(days < 0){ started = started.addDays(days);}
    int hours = started.secsTo(current)/3600;
    if(hours > 0){ started = started.addSecs(hours*3600); }
    int minutes = started.secsTo(current)/60;
    if(minutes > 0){ started = started.addSecs(minutes*60); }
    QString upTime = QString("Running: %1 days %2 hours %3 minutes %4 secs").arg(days).arg(hours).arg(minutes).arg(started.secsTo(current));
    
    TasObject& starTime = container.addNewObject("2", "startTime", "QDateTime");
    starTime.addAttribute("startTime", QDateTime::fromMSecsSinceEpoch(mStartTime).toString("dd.MM.yyyy hh:mm:ss"));
    starTime.addAttribute("upTime", upTime);
    


    QByteArray xml;
    model->serializeModel(xml);    
    delete model;
    response.setData(xml);
}
void ListAppsService::listRunninProcesses(TasResponse& response)
{
    TasDataModel* model = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = model->addNewObjectContainer(1, qtVersion, "qt");
    TasObject& apps = container.addNewObject(0, "processList", "processList");
    TasNativeUtils::runningProcesses(apps);
    QByteArray xml;
    model->serializeModel(xml);    
    delete model;
    response.setData(xml);
}
