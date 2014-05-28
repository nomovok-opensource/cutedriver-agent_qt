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

#include <QTimer>

#include <taslogger.h>
#include <windows.h>
#include <commctrl.h>
#include "uicommandservice.h"

UiCommandService::UiCommandService() {}
UiCommandService::~UiCommandService() {}

bool UiCommandService::executeService(TasCommandModel& model, TasResponse& response)
{    
    Q_UNUSED(response);
    
    if (model.service() == serviceName()) {
        foreach (TasTarget* target, model.targetList()) {
            foreach (TasCommand* command, target->commandList()) {
                if (command->name() == "PressEnter") {
                    if(!command->parameter("interval").isEmpty()){
                        int interval = command->parameter("interval").toInt();       
                        QTimer::singleShot(interval, this, SLOT(pressEnter()));
                    }
                    else{
                        pressEnter();
                    }
                    break;
                }
            }
        }
        return true;
    }
    else {
        return false;
    }
}

void UiCommandService::performTapCommand(TasCommand* command)
{
   // get x, y, time_to_hold as command parameters and click the screen at x,y
}

void UiCommandService::pressEnter(){
    TasLogger::logger()->debug("UiCommandService::pressEnter");
    keybd_event(VK_RETURN,0x1C,0,0);
}
