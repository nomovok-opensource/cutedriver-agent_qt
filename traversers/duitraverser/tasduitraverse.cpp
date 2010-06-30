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
 

//#include "duislider.h"
//#include "duibutton.h"
//#include "duitextedit.h"
//#include "duilabel.h"
//#include "duiimage.h"
//#include "duiapplethandle.h"
//#include "duiapplication.h"
#include <QtPlugin>
#include <QDebug>
#include <taslogger.h>
#include "testabilityutils.h"
#include "tasduitraverse.h"

Q_EXPORT_PLUGIN2(duitraverse, TasDuiTraverse)

/*!
    \class TasDuiTraverse
    \brief TasDuiTraverse traverse DUI components for adding custom details to them
        
    Using standard qt objects it is not always possible to get specific details from the 
    components. This TasHelperInterface implementation will allow us to add details from 
    the wanted dui components that are not accesible through the property or other
    generic way. 
*/

/*!
    Constructor
*/
TasDuiTraverse::TasDuiTraverse(QObject* parent)
    :QObject(parent)
{}

/*!
    Destructor
*/
TasDuiTraverse::~TasDuiTraverse()
{}

/*!
        
    Retuns false so that normal traverse is not changed.

*/
void TasDuiTraverse::traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command)
{
    Q_UNUSED(command);
//     TasLogger::logger()->debug("TasDuiTraverse::traverseObject in: " + object->objectName() + " class: " + 
//                                "TasDuiTraverse::traverseObject class: " + 
//                                QString(object->metaObject()->className()));

    //this class only traverses menu items
    if (object->inherits("DuiAppletHandle") ||
        object->inherits("MExtensionHandle")) {
        TasLogger::logger()->debug("TasDuiTraverse::traverseObject traversing DuiAppletHandle");
        QGraphicsItem* graphicsItem = qobject_cast<QGraphicsItem*>(object);

        TasLogger::logger()->debug("TasDuiTraverse::traverseObject Checking for rect");
        if (graphicsItem) {
            QRectF sceneRect = graphicsItem->sceneBoundingRect();        
            TasLogger::logger()->debug("TasDuiTraverse::traverseObject rect is " + 
                                       QString::number(sceneRect.x()) + ":" + 
                                       QString::number(sceneRect.y()));
        }
        TasObject& obj = objectInfo->addObject();            
        obj.setId(0);
        obj.setType("TDriverRef");    
        obj.setName("TDriverRef");
        TasLogger::logger()->debug("TasDuiTraverse::traverseObject adding applet handle to " + 
                                   object->property("pid").toString());

        obj.addAttribute("uri", object->property("pid").toString());
    }
    else if (object->inherits("QApplication")){
        TasLogger::logger()->debug("TasDuiTraverse::traverseObject traversing QApplication " + 
            TestabilityUtils::getApplicationName());
        QApplication* application = qobject_cast<QApplication*>(object);
        if (application){
            if (TestabilityUtils::getApplicationName() == "webwidgetrunner") {
                // TDriverRef to the actual program (wrtdaemon)
                TasObject& obj = objectInfo->addObject();
                obj.setId(0);
                obj.setType("TDriverRef");    
                obj.setName("TDriverRef");
                TasLogger::logger()->debug("_Reading property pid: " + application->property("pid").toString());
                obj.addAttribute("uri", application->property("pid").toString());
                obj.addAttribute("runtimeId", application->property("runtimePid").toString());
            }
        }
    }
    else if (QString(object->metaObject()->className()) == "WRT::Maemo::WebWidgetView") { 
        // We don't actually have the webwidget view headers, so going blind
        TasLogger::logger()->debug(">>TasDuiTraverse::traverseObject traversing WRT::Maemo::WebWidgetView ");

        quint64 pid = 0;
        quint64 runtimeid = 0;
        QMetaObject::invokeMethod(object, "daemonPid",
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(quint64, pid));
        QMetaObject::invokeMethod(object, "runtimeId",
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(quint64, runtimeid));

        TasObject& obj = objectInfo->addObject();
        obj.setId(0);
        obj.setType("TDriverRef");    
        obj.setName("TDriverRef");
        TasLogger::logger()->debug("TasDuiTraverse::traverseObject got pid: " + QString::number(pid));
        TasLogger::logger()->debug("TasDuiTraverse::traverseObject got runtimeid: " + QString::number(runtimeid));


        obj.addAttribute("uri", QString::number(pid));
        obj.addAttribute("runtimeId", QString::number(runtimeid));
    }
    
    
    
//     TasLogger::logger()->debug("TasDuiTraverse::traverseObject out");
}

/*!

    Adds deatails about QGraphicsItem based components. The components must implement 
    the QGraphicsItem::type() to allow succesfull casting. Will always return false.

*/
void TasDuiTraverse::traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem,TasCommand*)
{
    Q_UNUSED(objectInfo);
    Q_UNUSED(graphicsItem);
}
