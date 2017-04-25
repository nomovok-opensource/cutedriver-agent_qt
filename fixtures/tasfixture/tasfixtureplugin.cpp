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
#include <QGraphicsItem>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QtTest/qtestspontaneevent.h>

#include "tasfixtureplugin.h"
#include "testabilityutils.h"
#include "taslogger.h"


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
        QWindow* appWindow = TestabilityUtils::getApplicationWindow();
        if (appWindow) {
            QSize newSize(appWindow->height(), appWindow->width());
            appWindow->resize(newSize);
            result =  true;
        } else {
            QWidget* appWidget = TestabilityUtils::getApplicationWidget();

            if (appWidget) {
                QSize newSize(appWidget->height(), appWidget->width());
                appWidget->resize(newSize);
                result =  true;
            } else {
                stdOut = "Could not find application window.";
                result =  false;
            }
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
    else if(actionName == "ensureQmlVisible"){
        TasLogger::logger()->debug("TasFixturePlugin::execute ensureQmlVisible");
        if(parameters.value(OBJECT_TYPE) == GRAPHICS_ITEM_TYPE ){
            QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);
            if(item){
                QGraphicsItem* parentItem = item->parentItem();
                while(parentItem){
                    QGraphicsObject* parentObject = parentItem->toGraphicsObject();
                    if(parentObject && parentObject->inherits("QDeclarativeFlickable")){
                        //first check is the item already visibile (inside the flickable visible area)
                        QVariant variant = parentObject->property("visibleArea");
                        if(variant.isNull()){
                            TasLogger::logger()->debug("TasFixturePlugin::execute ensureQmlVisible flickable has no fucking visible area ");
                        }
                        //try to get the are from property
                        QObject* visibleArea = qvariant_cast<QObject*>(variant);
                        //if fails look from children
                        if(!visibleArea){
                            foreach(QObject *o, parentObject->children()){
                                if(o->inherits("QDeclarativeFlickableVisibleArea")){
                                    visibleArea = o;
                                    break;
                                }
                            }
                        }
                        if(visibleArea){
                            qreal contentX = parentObject->property("contentX").toReal();
                            qreal contentY = parentObject->property("contentY").toReal();
                            qreal visibleWidth = parentObject->property("contentWidth").toReal()*
                                visibleArea->property("widthRatio").toReal();
                            qreal visibleHeight = parentObject->property("contentHeight").toReal()*
                                visibleArea->property("heightRatio").toReal();

                            if(item->x() < contentX || item->x() > (contentX + visibleWidth)){
                                parentObject->setProperty("contentX", QVariant(item->x()));
                            }

                            if(item->y() < contentY || item->y() > (contentY + visibleHeight)){
                                parentObject->setProperty("contentY", QVariant(item->y()));
                            }
                            break;
                        }
                        else{
                            TasLogger::logger()->warning("TasFixturePlugin::execute ensureQmlVisible flickable visibleArea not found?");
                        }
                    }
                    parentItem = parentItem->parentItem();
                }
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
    else if(actionName == "logProperty" ){
        QObject* o = castToObject(objectInstance, parameters.value(OBJECT_TYPE));
        if(o){
            result = mLogger.startPropertyLog(o, parameters, stdOut);
        }
    }
    else if(actionName == "logPropertyResults" ){
        QObject* o = castToObject(objectInstance, parameters.value(OBJECT_TYPE));
        if(o){
            stdOut.clear();
            result = mLogger.getLogData(o, parameters, stdOut);
        }
    }
    else if(actionName == "stopLogProperty" ){
        QObject* o = castToObject(objectInstance, parameters.value(OBJECT_TYPE));
        if(o){
            stdOut.clear();
            result = mLogger.getLogData(o, parameters, stdOut);
            mLogger.stopLogger(o, parameters);
        }
    }
    else{
        stdOut = "The execution was ok. Parameters were { action: " + actionName ;
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
        result =  true;
    }
    return result;
}

QObject* TasFixturePlugin::castToObject(void* objectInstance, const QString& type)
{
    if(type == WIDGET_TYPE ){
        QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
        return widget;
    }
    else if(type == GRAPHICS_ITEM_TYPE ){
        QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);
        return item->toGraphicsObject();
    }
    else if(type == APPLICATION_TYPE ){
        return QCoreApplication::instance();
    }
    return 0;
}
