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
 

#include <tasqtdatamodel.h>
#include <taslogger.h>

#include "shellcommandservice.h"


ShellCommandService::ShellCommandService()
{
}

ShellCommandService::~ShellCommandService()
{
}

bool ShellCommandService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        TasCommand* command = getCommandParameters(model, "shellCommand");
        if(command && !command->text().isEmpty()){
            if (command->parameter("detached") == "true"){
                detachedShellCommand(command->text(), response);
            }
            else{
                shellCommand(command->text(), response);
            }
        }
        else{
            response.setErrorMessage(NO_COMMAND_TO_EXECUTE);
        }
        return true;
    }
    else{
        return false;
    }
}


/*!

Executes a command as a process
Output written to given socket as response.
*/
void ShellCommandService::shellCommand(QString message, TasResponse& response)
{
    TasLogger::logger()->debug("ShellCommandService::shellCommand: " + message);

    QProcess process;
    process.setReadChannelMode(QProcess::MergedChannels);
    process.setEnvironment(QProcess::systemEnvironment());
    process.start(message);
    
    process.closeWriteChannel();
    process.waitForFinished(4000);

    QByteArray output = process.readAll();
    response.setData(output);
}

/*!

Executes a command as a process
Output written to given socket as response.
*/
void ShellCommandService::detachedShellCommand(QString message, TasResponse& response)
{
    TasLogger::logger()->debug("ShellCommandService::detachedShellCommand: " + message);
    QProcess process;
    process.setReadChannelMode(QProcess::MergedChannels);
    process.setEnvironment(QProcess::systemEnvironment());
    bool started = process.startDetached(message);

    if(!started){
        response.setErrorMessage("Failed to start process!");
    }
}
