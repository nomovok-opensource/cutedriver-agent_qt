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
 
 

#include "actionhandler.h"

#include <QAction>
#include <QToolBar>

/*!
  \class ActionHandler
  \brief ActionHandler attempts to cause action trigger or hover

*/    


ActionHandler::ActionHandler()
{}

ActionHandler::~ActionHandler()
{}
  
bool ActionHandler::executeInteraction(TargetData data)
{
    QString commandName = data.command->name();
    if(commandName == "Hover" || commandName == "Trigger"){
        performActionEvent(*data.command, data.target);
        return true;
    }
    else{
        return false;
    }
}

void ActionHandler::performActionEvent(TasCommand& command, QWidget* widget)
{
    QString id = command.parameter("id");
    QString name = command.name();                    
    QAction* action = getAction(widget, id.toInt());    
    QWidget* actionWidget = widget;
    if(action){                        
        //only way to activate the action is to simulate mouse events
        //attempt to get the coordinates for the action...
        QPoint point(0,0); 
        if(widget->metaObject()->className() == QMenuBar::staticMetaObject.className()){
            QMenuBar* bar = qobject_cast<QMenuBar*>(widget);                    
            QRect rect = bar->actionGeometry(action);
            point = rect.topLeft();
        }
        else if(widget->metaObject()->className() == QMenu::staticMetaObject.className()){
            QMenu* menu = qobject_cast<QMenu*>(widget);
            QRect rect = menu->actionGeometry(action);
            point = rect.topLeft();
        }   
        else if(widget->metaObject()->className() == QToolBar::staticMetaObject.className()){
            QToolBar* bar = qobject_cast<QToolBar*>(widget);                
            actionWidget = bar->widgetForAction(action);
        }
        else{
            //skip since we are not sure what to do...
            return;
        }
        QPoint screenPoint = actionWidget->mapToGlobal(point);
        //add mouse events tha match the action type
        if ( name == "Hover"){             
            mMouseGen.doMouseMove(actionWidget, screenPoint);
        }
        else{
            mMouseGen.doMousePress(actionWidget, Qt::LeftButton, screenPoint);                
            mMouseGen.doMouseRelease(actionWidget, Qt::LeftButton, screenPoint);                
        }                   
    }
}
/*!
 
    Get the correct action associated with the widget and calculate its center pos.
 
 */
QAction* ActionHandler::getAction(QWidget* widget, int id)
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
