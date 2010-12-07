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
             
#include "startappservice.h"


#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE)) 
#include <windows.h>
#endif

const char* const DETACH_MODE = "detached";
const char* const SET_PARAMS_ONLY = "set_params_only";
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
    else if(arguments.contains(DETACH_MODE)){
        arguments.removeAll(DETACH_MODE);
        bool noWait = arguments.contains(NO_WAIT);
        arguments.removeAll(NO_WAIT);
        launchDetached(applicationPath, arguments, response, noWait);
    }
    else{
        launchAttached(applicationPath, arguments, response, parseEnvironmentVariables(command.parameter("environment")));   
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

void StartAppService::launchAttached(const QString& applicationPath,const QStringList& arguments, TasResponse& response, QHash<QString, QString>  environmentVariables)
//void StartAppService::launchAttached(const QString& applicationPath, const QStringList& arguments, TasResponse& response)
{
    TasLogger::logger()->debug("StartAppService::launchAttached in");
    QProcess *process = new QProcess();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QHashIterator<QString, QString> i(environmentVariables);
    while (i.hasNext()) {
        i.next();
        TasLogger::logger()->debug("StartAppService::launchAttached setting '" + i.key() +  "':'" + i.value()+"'");
        // Note that PATH seems to be special in Qt, and this will not work as expected
        // e.g. unsetting PATH will still start an app from the default PATH values
        env.insert(i.key(), i.value()); 
    }
    process->setProcessEnvironment(env);
    
    if(arguments.isEmpty() || arguments.join("").trimmed().isEmpty()){
        process->start(applicationPath);
    }
    else{
        process->start(applicationPath, arguments);
    }
   
    if(process->waitForStarted()){            
#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WINCE)) || defined(qdoc)        
        QString processId = QString::number(process->pid()).toLatin1().data();
#else                 
        QString processId = QString::number(process->pid()->dwProcessId).toLatin1().data();               
#endif   

        QString processName = TasCoreUtils::parseExecutable(applicationPath);
        new RegisterWaiter(response.requester(), TasClientManager::instance()->addClient(processId, processName, process), response.messageId());
        //response.setMessageDelayed(true);
    }          
    else{
        QString errorString = process->errorString();        
        TasLogger::logger()->error("TasServer::launchAttached: error: " + errorString);
        delete process;
        process = 0;
        response.setErrorMessage(errorString);
    }
    TasLogger::logger()->debug("StartAppService::launchAttached out");
}

void StartAppService::launchDetached(const QString& applicationPath, const QStringList& arguments, TasResponse& response, bool noWait)
{
    qint64 pid;
    if(QProcess::startDetached(applicationPath, arguments, ".", &pid)){
        QString processName = TasCoreUtils::parseExecutable(applicationPath);
        new RegisterWaiter(response.requester(), TasClientManager::instance()->addClient(QString::number(pid), processName),
                           response.messageId(), noWait);
    }
    else{
        TasLogger::logger()->error("TasServer::launchDetached: count not start the application " + applicationPath);
        response.setErrorMessage("Could not start the application " + applicationPath);
    }            
}

RegisterWaiter::RegisterWaiter(TasSocket* socket, TasClient *target, qint32 messageId, bool noWait)
{
    mMessageId = messageId;
    mSocket = socket;
    mProcessId = target->processId();
    mProcessName = target->applicationName();
    //TasLogger::logger()->debug("RegisterWaiter::RegisterWaiter " + mProcessId + " messageId " + QString::number(mMessageId));

    connect(target, SIGNAL(registered(const QString&)), this, SLOT(clientRegistered(const QString&)));       
    connect(target, SIGNAL(crashed()), this, SLOT(crashed()));
    mWaiter.setSingleShot(true);
    mWaiter.start(20000);

    connect(&mWaiter, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(mSocket, SIGNAL(socketClosed()), this, SLOT(socketClosed()));

    //maybe the app already is registered (e.g symbian only allows one instance of each app)
    //or wait not wanted (starting a native app)
    if(target->socket() || noWait ){
        clientRegistered(mProcessId);
    }
}

void RegisterWaiter::clientRegistered(const QString& processId)
 {
    TasLogger::logger()->debug("RegisterWaiter::clientRegistered " + processId);    
    //    TasLogger::logger()->debug("RegisterWaiter::clientRegistered respond with id " + QString::number(mMessageId));    
    mSocket->sendResponse(mMessageId, processId);
    mWaiter.stop();
    deleteLater();
}

void RegisterWaiter::timeout()
{
    TasLogger::logger()->error("RegisterWaiter::timeout. Application did not register in time. " + mProcessName + "(" + mProcessId + ")" );
    QString msg = "Application started ok but did not register. Maybe plugin is not loaded by the application!";
    mSocket->sendError(mMessageId, new QByteArray(msg.toUtf8()));
    deleteLater();
}

void RegisterWaiter::crashed()
{
    TasLogger::logger()->error("RegisterWaiter::crashed. Application crashed in startup. " + mProcessName + "(" + mProcessId + ")" );
    QByteArray* msg = new QByteArray(QString("The application " + mProcessName + " with Id " + mProcessId + " has crashed.").toUtf8());
    mSocket->sendError(mMessageId, msg);
    mWaiter.stop();
    deleteLater();
}

 

void RegisterWaiter::socketClosed()
{
    mWaiter.stop();
    TasLogger::logger()->error("RegisterWaiter::socketClosed. Connection to this waiter closed. Cannot respond. " );
    deleteLater();
}




