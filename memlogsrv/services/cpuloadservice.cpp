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

#include "cpuloadservice.h"
#include "cpuloadgenerator.h"
#include <taslogger.h>

CpuLoadService::CpuLoadService() 
{}

CpuLoadService::~CpuLoadService()
{
    delete mLoadGenerator;
}

bool CpuLoadService::executeService(TasCommandModel& model, TasResponse& response)
{
    TasLogger::logger()->debug("> CpuLoadService::executeService: " + model.service());
    bool status = false;
    
    if(model.service() == serviceName()) {
        status = true;
        int error = TAS_ERROR_NONE;
        TasCommand* command = 0;
        QString responseData;
        
        /*
         * Start process memory logging
         */
        if(commandIs(model, "", command)) {
            error = startLoad(*command);
            if (TAS_ERROR_NONE != error) {
                TasLogger::logger()->error("  Error in starting CPU load: " + QString::number(error));
                response.setErrorMessage("Error in starting CPU load: " + QString::number(error));
            }
        }
        /*
         * Stop process memory logging
         */
        else if (commandIs(model, "", command)) {
            error = stopLoad(*command, responseData);
            if (TAS_ERROR_NONE != error) {
                TasLogger::logger()->error("  Error in stopping CPU load: " + QString::number(error));
                response.setErrorMessage("Error in stopping CPU load: " + QString::number(error));
            }
            else {
                response.setData(responseData);
            }
        }
        else {
            TasLogger::logger()->error("  Unknown command: " + model.name());
            response.setErrorMessage("Unknown command: " + model.name());
        }
    }
    
    TasLogger::logger()->debug("< CpuLoadService::executeService");
    return status;
}

int CpuLoadService::startLoad(TasCommand& command)
{
    TasLogger::logger()->debug("> CpuLoadService::startLogging: " + command.name());
    
    QString loadInPercentage = command.parameter("load");
    if (loadInPercentage.isEmpty()) {
        return TAS_ERROR_PARAMETER;
    }
    
    bool ok = false;
    int load = loadInPercentage.toInt(&ok);
    int error = TAS_ERROR_NONE;
    if (ok) {
        mLoadGenerator = new CpuLoadGenerator();
        error = mLoadGenerator->start(load);
    }
    else {
        error = TAS_ERROR_PARAMETER;
    }
    
    TasLogger::logger()->debug("< CpuLoadService::startLogging");
    return error;
}

int CpuLoadService::stopLoad(TasCommand& command, QString& responseData)
{
    Q_UNUSED(responseData);
    TasLogger::logger()->debug("> CpuLoadService::stopLogging: " + command.name());

    int error = TAS_ERROR_NONE;
    if (mLoadGenerator) {
        error = mLoadGenerator->stop();
    }
    
    TasLogger::logger()->debug("< CpuLoadService::stopLogging");
    return error;
}

bool CpuLoadService::commandIs(
        TasCommandModel& commandModel, 
        const QString& commandName, 
        TasCommand*& command)
{
    TasTarget* target = commandModel.findTarget(APPLICATION_TARGET);
    if(target) {
        command = target->findCommand(commandName);
        if (command) {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}
