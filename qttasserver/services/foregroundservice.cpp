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
 * foregroundservice.cpp
 */

#include <taslogger.h>
#include <tascoreutils.h>
#include <tasdatashare.h>
#include "tasnativeutils.h"
#include "foregroundservice.h"

ForegroundService::ForegroundService() {}

ForegroundService::~ForegroundService() {}
    
bool ForegroundService::executeService(TasCommandModel& model, TasResponse& response)
{    
    bool status = false;
    if(model.service() == serviceName()) {
        status = true;
        TasCommand* command = getCommandParameters(model, "BringToForeground");
        if(command){
            // Find app from client manager
            TasClientManager* clientManager = TasClientManager::instance();
            TasClient* app = 0;
            app = clientManager->findByProcessId(command->parameter("pid"));
            
            if (app) {
                TasLogger::logger()->debug("   App found from client manager, PID: " + app->processId());
                // Bring to foreground using native utils
                int error = TasNativeUtils::bringAppToForeground(*app);
                if (TAS_ERROR_NONE == error) {
                    TasLogger::logger()->debug("   App brought to foreground");
                }
                else {
                    TasLogger::logger()->error("   Couldn't bring app to foreground, error: " + error);
                    response.setErrorMessage("   Couldn't bring app to foreground, error: " + error);
                }
            }
            else {
                TasLogger::logger()->error("   App not found from client manager, PID: " + model.id());
                response.setErrorMessage("   App not found from client manager, PID: " + model.id());
            }
        }
        else {
            TasLogger::logger()->error("   Unknown command: " + model.name());
            response.setErrorMessage("   Unknown command: " + model.name());
        }
    }
    return status;
}
