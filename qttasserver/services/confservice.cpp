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

#include <taslogger.h>

#include "confservice.h"
#include "version.h"

ConfService::ConfService()
{}

ConfService::~ConfService()
{}

/*!
  Passes service directed to plugins on to the correct plugin.
 */
bool ConfService::executeService(TasCommandModel& model, TasResponse& response)
{
    Q_UNUSED(response);
    if(model.service() == CONF_SERVICE){
        TasCommand* command = getCommandParameters(model, CONFIGURE_LOGGER_COMMAND);
        if(command){
            TasLogger::logger()->debug("ConfService::executeService configure logger");
            TasLogger::logger()->configureLogger(*command);
        }
        command = getCommandParameters(model, CONFIGURE_LOGEVENTS_COMMAND);
        if(command){
            TasLogger::logger()->configureEventLogger(*command);
        }
        return true;
    }    
    else if ( model.service() == VERSION_SERVICE ){

        // return TAS version as response
        response.setData( TAS_VERSION );

        return true;

    }
    else{
        return false;
    }
}

