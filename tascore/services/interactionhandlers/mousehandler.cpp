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
 
#include <QtTest/qtestspontaneevent.h>
#include "taslogger.h"
#include "mousehandler.h"

#include <tasdeviceutils.h>

/*!
  \class MouseHandler
  \brief MouseHandler generates mouse press and release events.

*/    


MouseHandler::MouseHandler()
{
    mCommands << "MousePress" << "MouseRelease" << "MouseClick" 
              << "Tap" << "TapScreen" << "MouseMove" <<  "MouseDblClick";
}

MouseHandler::~MouseHandler()
{
}
  
bool MouseHandler::executeInteraction(TargetData data)
{
    //TasLogger::logger()->debug("MouseHandler::executeInteraction");
    bool wasConsumed = false;
    TasCommand& command = *data.command;
    QString commandName = command.name();
    if(mCommands.contains(commandName)){       
        TapDetails details = makeDetails(data);
        setPoint(command, details);
        wasConsumed = true;        
        checkMoveMouse(command, details.point);        
        
        if (commandName == "MouseClick" || commandName == "Tap" || commandName == "TapScreen"){
            int count = 1;
            if(!command.parameter("count").isEmpty()){
                count = command.parameter("count").toInt();       
            }
            if(commandName == "TapScreen"){
                details.target = qApp->widgetAt(details.point.x(), details.point.y());
            }            
            if(details.target){
                int duration = command.parameter("duration").toFloat();

                if(command.parameter("interval").isEmpty()){
                    for(int i = 0 ; i < count; i++){
                        press(details);
                        if (duration != 0) {
                            TasCoreUtils::wait(duration);
                        }
                        release(details);
                     }                    
                }
                else{
                    //interval based tapping
                    int interval = command.parameter("interval").toInt();
                    new Tapper(this, details, count, interval);
                }

            }
        }
        else if(commandName == "MouseDblClick"){
            mMouseGen.doMouseDblClick(details.target, details.button, details.point);
        }   
        else if(commandName == "MousePress"){
            press(details);
        }
        else if(commandName == "MouseRelease"){
            release(details);
        }
        else if (commandName == "MouseMove"){                        
            move(details);
        }
    }
    else if (commandName == "Scroll"){
        wasConsumed = true;
        TapDetails details = makeDetails(data);

        int deltaValue = 0 ;
        Qt::Orientation direction = Qt::Vertical;
                   
        if (!command.parameter("delta").isEmpty()) {
            deltaValue = command.parameter("delta").toInt();
        }        
        if (!command.parameter("orientation").isEmpty()){
            if ( command.parameter("orientation") == ":horizontal"){
                direction = Qt::Horizontal;
            }
        }        
        mMouseGen.doScroll(details.target, details.point, deltaValue, details.button, direction);                
    }
    else if(commandName == "Hover" || commandName == "Trigger"){
        performActionEvent(makeDetails(data));
        wasConsumed = true;
    }
    return wasConsumed;
}

void MouseHandler::performActionEvent(TapDetails details)
{
    QString id = details.command->parameter("id");
    QWidget* actionWidget = details.target;
    QAction* action = getAction(actionWidget, id.toInt());    
    if(action){                        
        //only way to activate the action is to simulate mouse events
        //attempt to get the coordinates for the action...
        QPoint point(0,0);         
        if(QMenuBar* bar = qobject_cast<QMenuBar*>(actionWidget)){            
            QRect rect = bar->actionGeometry(action);
            point = rect.topLeft();
        }
        else if(QMenu* menu = qobject_cast<QMenu*>(actionWidget)){           
            QRect rect = menu->actionGeometry(action);
            point = rect.topLeft();
        }   
        else if(QToolBar* bar = qobject_cast<QToolBar*>(actionWidget)){
            actionWidget = bar->widgetForAction(action);
        }
        else{
            //skip since we are not sure what to do...
            return;
        }
        QPoint screenPoint = actionWidget->mapToGlobal(point);
        //add mouse events tha match the action type
        if ( details.command->name() == "Hover"){             
            move(details);
        }
        else{
            press(details);
            release(details);
        }                   
    }
}

/*!
    Get the correct action associated with the widget and calculate its center pos.
 */
QAction* MouseHandler::getAction(QWidget* widget, int id)
{
    QAction* action = NULL;
    QList<QAction*> actions = widget->actions();        
    if(actions.size() > 0){        
        QAction* target = (QAction*)id;    
        for(int i = 0 ; i < actions.size(); i++){
            QAction * ac = actions.at(i);            
            if ( ac == target){
                action = ac; 
                break;
            }            
        }
    }
    return action;
}

MouseHandler::TapDetails MouseHandler::makeDetails(TargetData data)
{
    TasCommand& command = *data.command;
    TapDetails details;
    details.command = data.command;
    details.target = data.target;
    details.point = data.targetPoint;
    details.identifier = TasCoreUtils::pointerId(data.targetItem);
    details.button = getMouseButton(command);
    details.pointerType = MouseHandler::TypeMouse;
    if(!command.parameter(POINTER_TYPE).isEmpty()){
        details.pointerType = static_cast<MouseHandler::PointerType>(command.parameter(POINTER_TYPE).toInt());
    }
    mMouseGen.setUseTapScreen(command.parameter("useTapScreen") == "true");  
            
    return details;
}

void MouseHandler::press(TapDetails details)
{
    if(details.pointerType == TypeMouse || details.pointerType == TypeBoth){
        mMouseGen.doMousePress(details.target, details.button, details.point);
    }
    if(details.pointerType == TypeTouch || details.pointerType == TypeBoth){        
        //set primary only when mouse events are sent
        bool primary  = (details.pointerType == TypeBoth);
        mTouchGen.doTouchBegin(details.target, details.point, primary, details.identifier);
    }
}
void MouseHandler::move(TapDetails details)
{
    if(details.pointerType == TypeMouse || details.pointerType == TypeBoth){
        mMouseGen.doMouseMove(details.target, details.point, details.button);
    }
    if(details.pointerType == TypeTouch || details.pointerType == TypeBoth){
        bool primary  = (details.pointerType == TypeBoth);
        mTouchGen.doTouchUpdate(details.target, details.point, primary, details.identifier);
    }
}
void MouseHandler::release(TapDetails details)
{
    if(details.pointerType == TypeMouse || details.pointerType == TypeBoth){
        mMouseGen.doMouseRelease(details.target, details.button, details.point);
    }
    if(details.pointerType == TypeTouch || details.pointerType == TypeBoth){
        bool primary  = (details.pointerType == TypeBoth);
        mTouchGen.doTouchEnd(details.target, details.point, primary, details.identifier);
    }
}


Qt::MouseButton MouseHandler::getMouseButton(TasCommand& command)
{
    Qt::MouseButton btn = Qt::NoButton;
    Qt::MouseButtons buttons = qApp->mouseButtons();
    QString button = command.parameter("button");
    if (!button.isEmpty()){         
        btn = static_cast<Qt::MouseButton>(button.toInt());
    }        
    else if(buttons & Qt::LeftButton){
        btn = Qt::LeftButton;
    }
    else if(buttons & Qt::RightButton){
        btn = Qt::RightButton;
    }
    return btn;
}

void MouseHandler::checkMoveMouse(TasCommand& command, QPoint point)
{
    if(command.parameter("mouseMove") == "true"){                
        mMouseGen.moveCursor(point);
    }           
}

/*!
 Set point from parameters. 
 */
void MouseHandler::setPoint(TasCommand& command, TapDetails& details)
{
    if(command.parameter("useCoordinates") == "true"){ 
        int x = command.parameter("x").toInt();
        int y = command.parameter("y").toInt();
        details.point.setX(x);
        details.point.setY(y);           
        details.identifier.append(QString::number(details.point.x()) +"_"+ QString::number(details.point.y()));
    }
}

Tapper::Tapper(MouseHandler* handler, MouseHandler::TapDetails details, int count, int interval)
{
    mHandler = handler;
    mMaxCount = count;
    mTapCount = 0;
    mDetails = details;
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(tap()));
    mTimer.start(interval);
}

void Tapper::tap()
{
    mTapCount++;
    
    mHandler->press(mDetails);
    mHandler->release(mDetails);

    if( mTapCount >= mMaxCount ){
        mTimer.stop();        
        deleteLater();
    }
}
