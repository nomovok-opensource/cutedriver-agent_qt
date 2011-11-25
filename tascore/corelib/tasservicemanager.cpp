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
            
#include <QMutableListIterator>
#include <QCoreApplication>

#include "tasservicemanager.h"

#include "tascommandparser.h"
#include "tasconstants.h"
#include "taslogger.h"

static QString NO_SERVICE = "The request did not specify a service!.";

/*!
  \class TasServiceManager
  \brief TasServiceManager manages the service commands used by qttas components.
    
  TasServiceManager is the manager for the commands in the service architecture
  used by qttas components. The service requests are implemented using a relatively
  standard form of the chain of responsibility pattern. TasServiceManager class
  takes care of the command execution and management. The command implementations
  only need to concern with the actual command implementation. 

  Commands which are reqistered to the manager will be invoked on all 
  service requests untill on command consumed the request. This is done
  by returning "true" from the execute method.

*/

/*!
  Construct a new TasServiceManager
 */
TasServiceManager::TasServiceManager()
{}

/*!
  Destructor. Destroys all of the reqistered commands.
 */
TasServiceManager::~TasServiceManager()
{
    qDeleteAll(mCommands);
    mCommands.clear();
}

/*!
  Register a command to the TasServiceManager.
 */
void TasServiceManager::registerCommand(TasServiceCommand* command)
{
    mCommands.append(command);
}

/*!
  Slot to listen to service requests coming from clients connecting 
  to a QtTas component using the TasSocket connectivity.
  The command xml is parsed into to the TasQtCommandData format
  and passed on the the commands in the command chain. In case no one
  is interested in the command an error message is written to the 
  client as a response.
*/
void TasServiceManager::serviceRequest(TasMessage& request, TasSocket* requester)
{
    //https://bugreports.qt.nokia.com/browse/QTBUG-21928
    QCoreApplication::instance()->processEvents(QEventLoop::DeferredDeletion);
    QString errorMessage;
    TasCommandModel* commandModel = parseMessageString(request.dataAsString(), errorMessage);
    if(commandModel){
        handleServiceRequest(*commandModel, requester, request.messageId());
    }
    else{
        TasResponse response(request.messageId());
        response.setErrorMessage(errorMessage);
        requester->sendMessage(response);
    }
    delete commandModel;
}

void TasServiceManager::handleServiceRequest(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId)
{
    TasLogger::logger()->debug("TasServiceManager::handleServiceRequest " + commandModel.service());
    TasResponse response(responseId);
    response.setRequester(requester);
    performService(commandModel, response);
    requester->sendMessage(response);
}

void TasServiceManager::performService(TasCommandModel& commandModel, TasResponse& response)
{
    TasLogger::logger()->debug("TasServiceManager::performService: " + commandModel.service());
    bool wasConsumed = false;
#ifdef Q_OS_SYMBIAN
    int err = 0;
    //run under symbian TRAP harness
    TRAP(err, wasConsumed = doServiceExecution(commandModel, response));
    if(err) {
        response.setData(serviceErrorMessage()+commandModel.service() + "\n## Symbian error code(" + QString::number(err) + ")\n");
        response.setIsError(true);
        wasConsumed = true;
    }
#else
    wasConsumed = doServiceExecution(commandModel, response);
#endif
    if(!wasConsumed){
        TasLogger::logger()->warning("TasServiceManager::executeCommand unknown service");
        response.setData(serviceErrorMessage()+commandModel.service());
        response.setIsError(true);
    }
}


bool TasServiceManager::doServiceExecution(TasCommandModel& commandModel, TasResponse& response)
{
    QMutableListIterator<TasServiceCommand*> i(mCommands);
    while (i.hasNext()){
        TasServiceCommand* command = i.next();
        if(command->executeService(commandModel, response)){
            return true;
        }
    }
    return false;
}


/*!
  Parse and validate command model from the given data string. Will return null if the command model
  could not be parsed or is not valid. Use error message to determine the error.
  Ownership of the model is transferred.
*/
TasCommandModel* TasServiceManager::parseMessageString(const QString& messageBody, QString& errorMessage)
{
    //    TasCommandModel* commandModel = TasCommandParser::parseCommandXml(messageBody);       
    TasCommandModel* commandModel = TasCommandModel::makeModel(messageBody);
    if(!commandModel){
        TasLogger::logger()->fatal("TasServiceManager::parseMessageString could not parse message.");
        errorMessage = PARSE_ERROR;        
    }
    else if(commandModel && commandModel->service().isEmpty()){
        TasLogger::logger()->fatal("TasServiceManager::parseMessageString command model did not contain a service.");
        errorMessage = NO_SERVICE+commandModel->service();
        delete commandModel;
        commandModel = 0;
    }    
    return commandModel;
}






