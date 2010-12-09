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
#include <QTimer>
#include <QCoreApplication>

#include <taslogger.h>

#include "closeappservice.h"

CloseAppService::CloseAppService()
{}

CloseAppService::~CloseAppService()
{}

bool CloseAppService::executeService(TasCommandModel& model, TasResponse& response)
{    
    Q_UNUSED(response);
    if(model.service() == serviceName()){
        TasCommand* command = getCommandParameters(model, "Close");
        if(command){
            stopApplication(model, response);
            command = 0;
        }
        else{
            command = getCommandParameters(model, "Kill");
            if(command){
                QString applicationId = command->parameter("uid");    
                TasLogger::logger()->debug("CloseAppService::executeService kill " + applicationId);
                TasClientManager::instance()->removeClient(applicationId, true);   
                command = 0;
            }
            else{
                TasLogger::logger()->error("CloseAppService::executeService no Close/Kill command found!");
                response.setErrorMessage("Could not parse the Close command from the request.");
            }
        }
        return true;
    }
    else{
        return false;
    }
}

/*!
  Send a request to the client plugin to close it self. If the process was started using the
  server a given period is waited for the app to close. If close fails the process is killed
  if requested.
 */
void CloseAppService::stopApplication(TasCommandModel& model, TasResponse& response)
{
    TasCommand* command = getCommandParameters(model, "Close");
    QString applicationId = command->parameter("uid");    

    if(applicationId == "0"){
        TasLogger::logger()->info("CloseAppService::stopApplication server close requested.");       
        response.requester()->connect(response.requester(), SIGNAL(messageSent()), QCoreApplication::instance(), SLOT(quit()));   
        return;
    }
}
