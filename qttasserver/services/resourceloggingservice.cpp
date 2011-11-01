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
/*
 * resourceloggingservice.cpp 
 */
#include <QProcess>

#include <tasmessages.h>

#include "resourceloggingservice.h"
#include "taslogger.h"

static const QString SERVER_NAME = "qttasmemlog_srv";

ResourceLoggingService::ResourceLoggingService()
{}

ResourceLoggingService::~ResourceLoggingService()
{
}

bool ResourceLoggingService::executeService(TasCommandModel& model, TasResponse& response)
{
    TasLogger::logger()->debug("> ResourceLoggingService::executeService: " + model.service());
    bool status = false;
  
    if(model.service() == serviceName()) {
        status = true;
        //the server is not running so we need to start it (must be in path to work)
        qint64 pid;
        if(QProcess::startDetached(SERVER_NAME, QStringList(), ".", &pid)){       
            new MemLogServerWaiter(response.requester(), TasClientManager::instance()->addClient(QString::number(pid)), 
                                   model.sourceString(), response.messageId());
        }
        else{
            response.setErrorMessage("Could not start the "+SERVER_NAME+", make sure it is in path!");
        }
    }
   
    TasLogger::logger()->debug("< ResourceLoggingService::executeService");
    return status;
}

MemLogServerWaiter::MemLogServerWaiter(TasSocket* requester, TasClient *target, const QString& commandXml, qint32 messageId)
{
    mSocket = requester;
    mTarget = target;
    mMessageId = messageId;
    mCommandXml = commandXml;
    connect(mTarget, SIGNAL(registered(const QString&)), this, SLOT(clientRegistered(const QString&)));       
    connect(mTarget, SIGNAL(crashed()), this, SLOT(crashed()));
    mWaiter.setSingleShot(true);
    mWaiter.start(20000);

    connect(&mWaiter, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(mSocket, SIGNAL(socketClosed()), this, SLOT(socketClosed()));

    if(mTarget->socket()){
        clientRegistered("");
    }
}

MemLogServerWaiter::~MemLogServerWaiter()
{}

void MemLogServerWaiter::clientRegistered(const QString& processId)
{
    Q_UNUSED(processId)
    mWaiter.stop();
    TasLogger::logger()->debug("MemLogServerWaiter::clientRegistered send message");
    TasMessage message(REQUEST_MSG, false, QByteArray(mCommandXml.toUtf8()), mMessageId);
    mSocket->messageAvailable(message);
    deleteLater();
}

void MemLogServerWaiter::timeout()
{
    TasLogger::logger()->error("MemLogServerWaiter::timeout memlogsrv did not register.");
    mSocket->sendError(mMessageId, SERVER_NAME + " started but did not register. Cannot not log memory!");
    deleteLater();
}

void MemLogServerWaiter::crashed()
{
    TasLogger::logger()->error("MemLogServerWaiter::crashed");
    mSocket->sendError(mMessageId, SERVER_NAME + " has crashed.");
    mWaiter.stop();
    deleteLater();
}

void MemLogServerWaiter::socketClosed()
{
    mWaiter.stop();
    TasLogger::logger()->error("MemLogServerWaiter::socketClosed. Connection to this waiter closed. Cannot respond. " );
    deleteLater();
}



