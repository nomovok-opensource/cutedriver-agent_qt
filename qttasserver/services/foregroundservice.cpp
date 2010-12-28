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
            bool ok;
            quint64 processId = command->parameter("pid").toULongLong(&ok, 10);       
            if(processId != 0 && ok){
                int error = TasNativeUtils::bringAppToForeground(processId);
                if (TAS_ERROR_NONE == error) {
                    TasLogger::logger()->debug("   App brought to foreground");
                }
                else {
                    TasLogger::logger()->error("   Couldn't bring app to foreground, error: " + error);
                    response.setErrorMessage("   Couldn't bring app to foreground, error: " + error);
                }
            }
            else{
                TasLogger::logger()->error("Invalid pid: " + model.id());
                response.setErrorMessage("Invalid pid: " + model.id());
            }
        }
        else {
            TasLogger::logger()->error("   Unknown command: " + model.name());
            response.setErrorMessage("   Unknown command: " + model.name());
        }
    }
    else if (model.service() == "changeOrientation" ){
        status = true;
        TasCommand* command = getCommandParameters(model, "ChangeOrientation");
        if(command){
            TasNativeUtils::changeOrientation(command->parameter("direction"));
        }
    }
    return status;
}
