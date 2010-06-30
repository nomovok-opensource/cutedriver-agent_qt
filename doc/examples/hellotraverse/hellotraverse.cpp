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
 

#include <QtPlugin>
#include <QDebug>

#include <QGraphicsWidget>

#include "hellotraverse.h"

#include <taslogger.h>
#include <tasqtcommandmodel.h>

#include <testabilityutils.h>
#include <tasuitraverser.h>
#include <tastraverserloader.h>

Q_EXPORT_PLUGIN2(hellotraverse, HelloTraverse)

/*!
    \class HelloTraverse
    \brief HelloTraverse traverse web kit
        
*/

/*!
    Constructor
*/
HelloTraverse::HelloTraverse(QObject* parent)
    :QObject(parent)
{
    TasLogger::logger()->debug("HelloTraverse loaded!");
}

/*!
    Destructor
*/
HelloTraverse::~HelloTraverse()
{}

/*!
  Traverse graphicsitem(widget) 
*/
void HelloTraverse::traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command)
{
    Q_UNUSED(objectInfo);
    Q_UNUSED(graphicsItem);
    Q_UNUSED(command);
    
    // Looking for a graphics item?
}



/*!
  Traverse qobjects(widget)
*/
void HelloTraverse::traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command)
{    
    // These logger events will go to /logs/testability/appname.log,
    // if the directory exists and is user writable
    // Logging something here will create a lot of log entries, as all qobjects are traversed here
    TasLogger::logger()->debug("HelloTraverse::traverseObject in: " + 
                               object->objectName() + 
                               " \n Class: " +
                               QString(object->metaObject()->className()));
    TasLogger::logger()->debug("HelloTraverse::traverseObject app name: " + 
                               TestabilityUtils::getApplicationName());

    // A Traverser traverses all objects, normally but we are only interested in a certain object
    // Here we look for something to a specific e.g. MLabel..
    if (object->inherits("QObject")) {
        // But For example sake, add a property to all QObjects
        
        // TasObject is the data object that contains all properties for each element
        // Let's add something, this value should be seen in the UI State.
        objectInfo->addAttribute("hello", "Hello, world!");

        // Use objectInfo->addObject() to add sub objects if e.g. the tree is 
        // missing complete objects
        // TasObject obj = object->addObject();
        // Set the default mandatory values to the object
        // addObjectDetails(obj, myCustomObject);
    }

    TasLogger::logger()->debug("HelloTraverse::traverseObject: out");
}

 
