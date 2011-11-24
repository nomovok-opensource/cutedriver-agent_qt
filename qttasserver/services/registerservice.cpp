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


#include <QHash>

#include <tasqtdatamodel.h>
#include <taslogger.h>

#include "registerservice.h"


RegisterService::RegisterService()
{
}

RegisterService::~RegisterService()
{
}

bool RegisterService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        TasCommand* command = getCommandParameters(model, COMMAND_REGISTER);
        if(command){
            registerPlugin(*command, response);
            connect(response.requester(), SIGNAL(messageSent()), this, SLOT(registerQueuedClients()));
        }
        else{
            command = getCommandParameters(model, COMMAND_UNREGISTER);
            if(command){
                unRegisterPlugin(*command);
            }
        }
        return true;
    }
    else{
        return false;
    }
}

void RegisterService::registerPlugin(TasCommand& command, TasResponse& response)
{
    ClientDetails client;
    client.processId = command.parameter(PLUGIN_ID);
    client.processName = command.parameter(PLUGIN_NAME);
#ifdef Q_OS_SYMBIAN
    client.applicationUid = command.parameter(APP_UID);
#endif
    client.pluginType = command.parameter(PLUGIN_TYPE);
    client.socket = QWeakPointer<TasSocket>(response.requester());
    mClientQueue.enqueue(client);
}

void RegisterService::registerQueuedClients()
{
    TasLogger::logger()->debug("RegisterService::registerPlugin");
    disconnect(sender(), 0, this, 0);
    while (!mClientQueue.isEmpty()){
        ClientDetails client = mClientQueue.dequeue();
        if(client.socket.isNull()){
            TasLogger::logger()->warning("RegisterService::registerPlugin client "+ client.processName +" socket died before register was completed!");
            client.socket.clear();
        }        
        else{
            TasLogger::logger()->info("RegisterService::registerPlugin: register plugin with processId: "
                                      + client.processId + " name: " + client.processName +", type: "+
                                      client.pluginType);

#ifdef Q_OS_SYMBIAN
            TasClientManager::instance()->addRegisteredClient(client.processId, client.processName,
                                                              client.socket.data(), client.pluginType, client.applicationUid);
#else
            TasClientManager::instance()->addRegisteredClient(client.processId, client.processName, client.socket.data(), client.pluginType);
#endif
        }
        TasClientManager::instance()->detachFromStartupData(client.processName);
    }

}

void RegisterService::unRegisterPlugin(TasCommand& command)
{
    QString processId = command.parameter("processId");
    TasLogger::logger()->info("TasServer::serviceRequest: unregister plugin: " + processId);
    TasClientManager::instance()->removeClient(processId);
}

