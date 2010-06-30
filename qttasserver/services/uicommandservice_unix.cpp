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
 

#include "uicommandservice.h"
#include <taslogger.h>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>



/*!
  \class UiCommandService
  \brief UiCommandService Manages Generic UI Commands directed to the entire computer
*/    



UiCommandService::UiCommandService()
{
}

UiCommandService::~UiCommandService()
{
}


bool UiCommandService::executeService(TasCommandModel& model, TasResponse& response)
{    
    TasLogger::logger()->debug("UiCommandService::executeService " + model.service() );    


    
    Q_UNUSED(response);
    if(model.service() == serviceName() ){
        foreach (TasTarget* target, model.targetList()) {
            if (target->type() == "Application") {
                foreach (TasCommand* command, target->commandList()) {
                    if (command->name() == "TapScreen") {
                        performTapCommand(command);
                    }
                }
            }
        }
        return true;
    }
    else{
        return false;
    }
}




void UiCommandService::performTapCommand(TasCommand* command)
{
    TasLogger::logger()->debug("UiCommandService::performTapCommand " );
    int x  = command->parameter("x").toInt();
    int y  = command->parameter("y").toInt();
    int duration = 1000000*command->parameter("time_to_hold").toFloat();
    TasLogger::logger()->debug("UiCommandService::performTapCommand holding " + 
                               QString::number(duration));
     Display* dpy = 0;
     Window root = None;
     dpy = XOpenDisplay(NULL);       
     root = DefaultRootWindow(dpy);    
     if (!dpy) {
         // TODO ERROR messages?
         return;
     }

     // Move the Cursor to given coords
     XWarpPointer(dpy, None, root, 0, 0, 0, 0, 
                  x,y);
     XFlush(dpy);

     XTestFakeButtonEvent(dpy, Button1, true, CurrentTime);
     usleep(duration);
     XTestFakeButtonEvent(dpy, Button1, false, CurrentTime);
     XFlush(dpy);
     XCloseDisplay(dpy);

}

void UiCommandService::pressEnter()
{

}
