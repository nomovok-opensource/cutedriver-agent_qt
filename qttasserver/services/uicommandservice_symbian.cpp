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
 * uicommandservice_symbian.cpp
 */

#include <e32event.h>
#include <w32std.h>
#include <taslogger.h>
#include "uicommandservice.h"

UiCommandService::UiCommandService() {}
UiCommandService::~UiCommandService() {}

bool UiCommandService::executeService(TasCommandModel& model, TasResponse& response)
{    
    TasLogger::logger()->debug("-> UiCommandService::executeService " + model.service());    
    Q_UNUSED(response);
    
    if (model.service() == serviceName()) {
        foreach (TasTarget* target, model.targetList()) {
            if (target->type() == "Application") {
                foreach (TasCommand* command, target->commandList()) {
                    if (command->name() == "TapScreen") {
                        performTapCommand(command);
                    }
                }
            }
        }
        TasLogger::logger()->debug("<- UiCommandService::executeService");
        return true;
    }
    else {
    	TasLogger::logger()->debug("<- UiCommandService::executeService");
        return false;
    }
}

void UiCommandService::performTapCommand(TasCommand* command)
{
    TasLogger::logger()->debug("-> UiCommandService::performTapCommand " + command->name());
    int x = command->parameter("x").toInt();
    int y = command->parameter("y").toInt();
    int duration = command->parameter("time_to_hold").toInt() * 1000;
    TasLogger::logger()->debug(
			"   Tapping in (" + QString::number(x) + 
			", " + QString::number(y) + 
			") for the duration of " + QString::number(duration));

	TPoint pos(x, y);
	TRawEvent eventDown;
	eventDown.Set(TRawEvent::EButton1Down, pos.iX, pos.iY);
	int error = UserSvr::AddEvent(eventDown);
	if (KErrNone != error) {
		TasLogger::logger()->error("   Error in EButton1Down: " + QString::number(error));
	}
	User::After(duration);
	TRawEvent eventUp;
	eventUp.Set(TRawEvent::EButton1Up, pos.iX, pos.iY);
	error = UserSvr::AddEvent(eventUp);
	if (KErrNone != error) {
		TasLogger::logger()->error("   Error in EButton1Up: " + QString::number(error));
	}
    
    TasLogger::logger()->debug("<- UiCommandService::performTapCommand " + command->name());
}


void UiCommandService::pressEnter()
{
}
