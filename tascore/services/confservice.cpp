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



#include "taslogger.h"

#include "confservice.h"

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
        TasTarget* target = model.findTarget(APPLICATION_TARGET);
        if(target){
            TasCommand* command = target->findCommand(CONFIGURE_LOGGER_COMMAND);
            if(command){
                TasLogger::logger()->configureLogger(*command);
            }
            command = target->findCommand(CONFIGURE_LOGEVENTS_COMMAND);
            if(command){
                TasLogger::logger()->configureEventLogger(*command);
            }
        }
        return true;
    }
    else{
        return false;
    }
}

