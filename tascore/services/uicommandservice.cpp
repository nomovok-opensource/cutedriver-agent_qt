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
 


#include <QApplication>
#include <QListIterator>

#include "tascommandparser.h"
#include "taslogger.h"
#include "uicommandservice.h"
#include "tassocket.h"

#include "actionhandler.h"
#include "gesturehandler.h"
#include "keyhandler.h"
#include "mousehandler.h"
#include "multitouchhandler.h"
#include "viewitemhandler.h"

#include "tasdeviceutils.h"

/*!
  \class UiCommandService
  \brief UiCommandService manages ui commands send to the app

*/    

UiCommandService::UiCommandService(QObject* parent)
    :QObject(parent)
{
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(executeNextCommand()));
    mInteractionHandlers.append(new MouseHandler());
    mInteractionHandlers.append(new KeyHandler());
    mInteractionHandlers.append(new ActionHandler());
    mInteractionHandlers.append(new GestureHandler());
    mInteractionHandlers.append(new MultitouchHandler());
    mInteractionHandlers.append(new ViewItemHandler());
}

UiCommandService::~UiCommandService()
{
    while (!mCommandQueue.isEmpty()){
        delete mCommandQueue.takeFirst();
    } 
    QMutableListIterator<InteractionHandler*> i(mInteractionHandlers);
    while (i.hasNext()){
        delete i.next();
    }
    mInteractionHandlers.clear();   
}

bool UiCommandService::executeService(TasCommandModel& model, TasResponse& response)
{    
    if(model.service() == serviceName() ){
        // Turn screen on.
        TasDeviceUtils::resetInactivity();
        parseValidTargets(model);
        mTimer.setInterval(model.interval());
        //start execution of commands after we have responded to the server
        connect(response.requester(), SIGNAL(messageSent()), this, SLOT(startTimer()));             
        return true;
    }
    else{
        return false;
    }
}

void UiCommandService::startTimer()
{
    disconnect(sender(), 0, this, 0); 
    mTimer.start();
}

void UiCommandService::parseValidTargets(TasCommandModel& model)
{
    TasTarget* target = 0;
    foreach(target, model.targetList()){
        if(model.isMultitouch()){
            TasLogger::logger()->debug("UiCommandService::parseValidTargets is multi");
            mMultiTouchCommands.append(new TasTarget(*target));
        }
        else{
            mCommandQueue.enqueue(new TasTarget(*target));
        }
    }
}

void UiCommandService::executeNextCommand()
{
    mTimer.stop();

    //1. check for multitouch operations
    if(!mMultiTouchCommands.isEmpty()){
        QList<TargetData> dataList;
        TasTarget* target;
        foreach(target, mMultiTouchCommands){
            TasCommand* command = 0;
            foreach(command, target->commandList()){
                TargetData data = makeInteractionData(target);
                if(data.target){
                    data.command = command;
                    dataList.append(data);
                }
            }
        }
        performMultitouchCommand(dataList);   
        //make sure the list is empty
        target = 0;
        foreach(target, mMultiTouchCommands){
            delete target;
        }
        mMultiTouchCommands.clear();
    }
    else if(!mCommandQueue.isEmpty()){
        //2. normal
        TasTarget* target = mCommandQueue.dequeue();   
        TargetData data = makeInteractionData(target);
        if(data.target){
            TasCommand* command = 0;
            foreach(command, target->commandList()){
                data.command = command;
                performCommand(data);
            }
        }
        delete target;
    }
    else{
        mTimer.stop();
    }

    //restart the timer if more commands left
    if(!mCommandQueue.isEmpty()){
        mTimer.start();
    }
    TasLogger::logger()->debug("UiCommandService::executeNextCommand done");
}

TargetData UiCommandService::makeInteractionData(TasTarget* commandTarget)
{
    QString targetType = commandTarget->type();
    quint32 id = commandTarget->id().toUInt();
    QWidget* target = 0;
    QGraphicsItem* item = 0;
    QPoint point;
    if(targetType == TYPE_GRAPHICS_VIEW){
        item = findGraphicsItem(id); 
        if(item){       
            target = viewPortAndPosition(item, point);   
        }
    }
    else if(targetType == TYPE_STANDARD_VIEW || targetType == TYPE_ACTION_VIEW){
        if(commandTarget->id() == "FOCUSWIDGET"){
            target = qApp->focusWidget();
        }
        else{
            target = findWidget(id);
        }
        if(target){                        
            point = target->mapToGlobal(target->rect().center());
        }
    }
    else if(targetType == TYPE_WEB){
        QListIterator<TasCommand*> j(commandTarget->commandList());
        if (j.hasNext()){        
            TasCommand* command = j.next();
            target = qApp->widgetAt(command->parameter("x").toInt(), command->parameter("y").toInt());
            if(!target) {
                TasLogger::logger()->warning("UiCommandService::performUiCommands target not found x:" + 
                                             command->parameter("x") + " y:" + command->parameter("y"));
            }
        }
    }
    else if (targetType == TYPE_APPLICATION_VIEW){
        target = getApplicationWindow();
        if(target){                        
            point = target->mapToGlobal(target->rect().center());
        }
    }
    else{
        TasLogger::logger()->warning("UiCommandService::performUiCommands unknown type");
    }
    TargetData data;
    data.target = target;
    data.targetItem = item;
    data.targetPoint = point;
    return data;
}
 


void UiCommandService::performCommand(TargetData data)
{
    QMutableListIterator<InteractionHandler*> i(mInteractionHandlers);
    while (i.hasNext()){
        InteractionHandler* commander = i.next();
        if(commander->executeInteraction(data)){
            break;
        }
    }
}


void UiCommandService::performMultitouchCommand(QList<TargetData> dataList)
{
    QMutableListIterator<InteractionHandler*> i(mInteractionHandlers);
    while (i.hasNext()){
        InteractionHandler* commander = i.next();
        if(commander->executeMultitouchInteraction(dataList)){
            break;
        }
    }
}
