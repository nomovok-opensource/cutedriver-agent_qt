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
 


#include "viewitemhandler.h"

#include <QTestEventList>
#include <QKeyEvent>

#include "taslogger.h"

/*!
  \class ViewItemHandler
  \brief ViewItemHandler generates keyboard events.

*/    


ViewItemHandler::ViewItemHandler()
{}

ViewItemHandler::~ViewItemHandler()
{}
  
bool ViewItemHandler::executeInteraction(TargetData data)
{
    QWidget* target = data.target;    
    TasCommand& command = *data.command;

    QString commandName = command.name();
    bool wasConsumed = false;
    if (commandName == "CheckState"){
        QTreeWidget* treeWidget = qobject_cast<QTreeWidget*>(target);
        if(treeWidget){
            setCheckState(treeWidget, command);
        }
    }
    return wasConsumed;
}

void ViewItemHandler::setCheckState(QTreeWidget* treeWidget, TasCommand& command)
{
    Qt::CheckState newState = static_cast<Qt::CheckState>(command.parameter("state").toInt());
    int column = command.parameter("column").toInt();
    //look for the item
    QTreeWidgetItem* item = findTreeItem(treeWidget, command.parameter("item").toUInt());
    if(item){
        item->setCheckState(column, newState);
    }
}

QTreeWidgetItem* ViewItemHandler::findTreeItem(QTreeWidget* treeWidget, quint32 itemId)
{
    QTreeWidgetItem* item = 0;
    int count = treeWidget->topLevelItemCount(); 
    for(int i = 0; i < count; i++){
        QTreeWidgetItem* candidate = treeWidget->topLevelItem(i);
        if( (quint32)candidate == itemId){
            item = candidate;
            break;
        }
        else{
            item = findFromTreeItem(candidate, itemId);
            if(item){
                break;
            }
        }
    }
    return item;
}

QTreeWidgetItem* ViewItemHandler::findFromTreeItem(QTreeWidgetItem* parent, quint32 itemId)
{
    QTreeWidgetItem* item = 0;
    int count = parent->childCount();
    for(int i = 0 ; i < count; i++){        
        QTreeWidgetItem* candidate = parent->child(i);
        if((quint32)candidate == itemId){
            item = candidate;
            break;
        }
        else{
            item = findFromTreeItem(candidate, itemId);
            if(item){
                break;
            }
        }       
    }
    return item;
}
