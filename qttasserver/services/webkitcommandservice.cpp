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
 

#include "webkitcommandservice.h"

#include <taslogger.h>

// #include <stdio.h>
// #include <stdlib.h>


/*!
  \class WebkitCommandService
  \brief WebkitCommandService Manages Generic UI Commands directed to the entire computer
*/    

WebkitCommandService::WebkitCommandService()
{
}

WebkitCommandService::~WebkitCommandService()
{
}

bool WebkitCommandService::executeService(TasCommandModel& model, TasResponse& response)
{    
    Q_UNUSED(response);
    if(model.service() == serviceName() ){
        foreach (TasTarget* target, model.targetList()) {
            //if (target->type() == "JavaScript") {
    			TasLogger::logger()->debug("WebkitCommandService::executeService type " + target->type() );

            	foreach (TasCommand* command, target->commandList()) {
                    if (command->name() == "RunJavaScriptOnObject") {
                    	executeJavaScript(command);
                    }
                }
            //}
        }
        return true;
    }
    else{
        return false;
    }
}



void WebkitCommandService::executeJavaScript(TasCommand* command)
{
    TasLogger::logger()->debug("WebkitCommandService::executeJavaScript \"" + command->parameter("java_script")  + "\"");

}

