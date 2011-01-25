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
#include <QCoreApplication>

#include <taslogger.h>
#include <tascoreutils.h>
#include <tasdatashare.h>

#include "tasdeviceutils.h"
#include "tasclientmanager.h"
             
#include "startappservice.h"


#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE)) 
#include <windows.h>
#endif

const char* const SET_PARAMS_ONLY = "set_params_only";
const char* const DETACH_MODE = "detached";
const char* const NO_WAIT = "noWait";



StartAppService::StartAppService()
{}

StartAppService::~StartAppService()
{}

bool StartAppService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        // Turn screen on.
        TasDeviceUtils::resetInactivity();

        TasCommand* command = getCommandParameters(model, "Run");
        if(command){
            startApplication(*command, response);
        }
        else{
            TasLogger::logger()->error("StartAppService::executeService no Run command found!");
            response.setErrorMessage("Could not parse Run command from the request!");
        }
        return true;
    }
    else{
        return false;
    }
}

/*!
  Attempts to start a process using the application path send in the command model.
 */
void StartAppService::startApplication(TasCommand& command, TasResponse& response)
{
    QString applicationPath = command.parameter("application_path");    
    QString args = command.parameter("arguments");
    TasLogger::logger()->debug("TasServer::startApplication: " + applicationPath);
    QStringList arguments = args.split(",");

    setRuntimeParams(command);

    if(arguments.contains(SET_PARAMS_ONLY)){
        // do not start app, just need to set the parameters
        response.requester()->sendResponse(response.messageId(), "0");
    }
    else{
        arguments.removeAll(DETACH_MODE);
        bool noWait = arguments.contains(NO_WAIT);
        arguments.removeAll(NO_WAIT);
        launchDetached(applicationPath, arguments, response, noWait);
    }
}


void StartAppService::setRuntimeParams(TasCommand& command)
{
    QString applicationPath = command.parameter("application_path");    
    QString eventList = command.parameter("events_to_listen");
    QString signalList = command.parameter("signals_to_listen");    
    TasLogger::logger()->error("StartAppService::setRuntimeParams signals: " + signalList);
    if(!eventList.isEmpty() || !signalList.isEmpty()){
        TasSharedData startupData(eventList.split(","), signalList.split(","));
        QString identifier = TasCoreUtils::parseExecutable(applicationPath);
        if(!TasClientManager::instance()->writeStartupData(identifier, startupData)){
            TasLogger::logger()->error("StartAppService::setRuntimeParams could not set run time params for identifier: " + identifier + "!");
        }
        else {
            TasLogger::logger()->error("StartAppService::setRuntimeParams set with identifier: " + identifier);
        }
    }
}


QHash<QString, QString> StartAppService::parseEnvironmentVariables(const QString& env) {
    QHash<QString,QString> vars;
    QStringList var = env.split(" ");
    foreach(QString str, var) {
        QStringList key = str.split("=");
        if (key.size() == 2) {
            vars[key.at(0)] = key.at(1);
        }
    }
    return vars;
}

void StartAppService::launchDetached(const QString& applicationPath, const QStringList& arguments, TasResponse& response, bool noWait)
{
    qint64 pid;
    if(QProcess::startDetached(applicationPath, arguments, ".", &pid)){
	TasClientManager::instance()->addStartedApp(applicationPath, QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        response.requester()->sendResponse(response.messageId(), QString::number(pid));   
    }
    else{
        TasLogger::logger()->error("TasServer::launchDetached: count not start the application " + applicationPath);
        response.setErrorMessage("Could not start the application " + applicationPath);
    }            
}

