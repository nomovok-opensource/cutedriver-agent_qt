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
#include <QHash>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QtTest/qtestspontaneevent.h>

#include "tasfixtureplugin.h"
#include "testabilityutils.h"
#include "taslogger.h"

Q_EXPORT_PLUGIN2(tasfixtureplugin, TasFixturePlugin)

/*!
  \class TasFixturePlugin
  \brief TasDuiTraverse traverse DUI components for adding custom details to them
        
  Using standard qt objects it is not always possible to get specific details from the 
  components. This TasHelperInterface implementation will allow us to add details from 
  the wanted dui components that are not accesible through the property or other
  generic way. 
*/

/*!
  Constructor
*/
    TasFixturePlugin::TasFixturePlugin(QObject* parent)
        :QObject(parent)
{}

/*!
  Destructor
*/
TasFixturePlugin::~TasFixturePlugin()
{}

/*!
  Implementation for traverse so always true.
*/
bool TasFixturePlugin::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    // place your own code below
    bool result = true;

    if(actionName.toLower() == "change_orientation"){
        QWidget* appWindow = TestabilityUtils::getApplicationWindow();
        if(appWindow){
            QSize newSize(appWindow->height(), appWindow->width());
            appWindow->resize(newSize);    
            result =  true;
        }
        else{
            stdOut = "Could not find application window.";
            result =  false;
        }
    }
    // set the stdOut if you wish to pass information back to Testtability Driver
    else if(actionName == "fail"){
        stdOut = "The execution failed. Parameters were {";
        result =  false;
    }
    else if(actionName == "contextMenu"){
        TasLogger::logger()->debug("TasFixturePlugin::execute contextMenu");
        QObject* target = 0;
        QPoint globalPoint(-1, -1);
        QPoint localPoint(-1, -1);
        if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
            TasLogger::logger()->debug("TasFixturePlugin::execute widget");
            QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
            if (widget) {
                localPoint = widget->rect().center();
                globalPoint = widget->mapToGlobal(localPoint);
                target = qobject_cast<QObject*>(widget);
            }
        }
        else if(parameters.value(OBJECT_TYPE) == GRAPHICS_ITEM_TYPE ){
            TasLogger::logger()->debug("TasFixturePlugin::execute graphicsitem");
            QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);
            if(item) {
                QWidget* widget = TestabilityUtils::viewPortAndPosition(item, globalPoint);
                localPoint = widget->mapFromGlobal(globalPoint);
                target = qobject_cast<QObject*>(widget);
            }
        }

        if(localPoint.x() > -1 && localPoint.y() > -1) {
            TasLogger::logger()->debug("TasFixturePlugin::execute local point x:" + QString().setNum(localPoint.x()) + " y:" + QString().setNum(localPoint.y()) + " global point x:" + QString().setNum(globalPoint.x()) + " y:" + QString().setNum(globalPoint.y()));
            //QEvent event = QContextMenuEvent(QContextMenuEvent::Other, point );
            //QEvent event = QContextMenuEvent(QContextMenuEvent::Mouse, localPoint, globalPoint );
            QContextMenuEvent e(QContextMenuEvent::Mouse, localPoint, globalPoint, 0);
            QSpontaneKeyEvent::setSpontaneous(&e);
            qApp->notify(target, &e);
        }
    }
    else if(actionName == "ensureVisible"){
        TasLogger::logger()->debug("TasFixturePlugin::execute ensureVisible");
        QWidget* widget = 0;
        QGraphicsItem* item = 0;

        if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
            TasLogger::logger()->debug("TasFixturePlugin::execute ensureVisible widget type");
            TasLogger::logger()->debug("TasFixturePlugin::execute widget");
            widget = reinterpret_cast<QWidget*>(objectInstance);
        }
        
        if (widget) {
            TasLogger::logger()->debug("TasFixturePlugin::execute widget != null");
            item = reinterpret_cast<QGraphicsItem*>(widget);
        } else {
            TasLogger::logger()->debug("TasFixturePlugin::execute widget == null");
            item = reinterpret_cast<QGraphicsItem*>(objectInstance);
        }

        if (item) {
            TasLogger::logger()->debug("TasFixturePlugin::execute item != null");
            QGraphicsView* view = TestabilityUtils::getViewForItem(item);
            if (view) {
                view->ensureVisible(item);
            }
            else {
                TasLogger::logger()->debug("TasFixturePlugin::execute view == null");
            }
        }
    }
    else if(actionName == "setFocus"){
		if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
			QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
			if(widget){
				widget->setFocus(Qt::MouseFocusReason);
			}
		}
		else if(parameters.value(OBJECT_TYPE) == GRAPHICS_ITEM_TYPE ){
			QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);  
			if(item){
				item->setFocus(Qt::MouseFocusReason); 
			}
		}
	}

    else{
        stdOut = "The execution was ok. Parameters were {";
        result =  true;
    }
    QHash<QString, QString>::const_iterator i = parameters.constBegin();
    while (i != parameters.constEnd()) {
        stdOut.append("(");
        stdOut.append(i.key());
        stdOut.append("=>");
        stdOut.append(i.value());
        stdOut.append(")");
        ++i;
        
        stdOut.append("}");
        // set the return value as boolean
    }
    return result;
}
