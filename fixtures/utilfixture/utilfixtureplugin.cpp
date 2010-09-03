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
#include "utilfixtureplugin.h"
#include "testabilityutils.h"
#include "taslogger.h"

Q_EXPORT_PLUGIN2(utilfixtureplugin, UtilFixturePlugin)

/*!
  \class UtilFixturePlugin
  \brief TasDuiTraverse traverse DUI components for adding custom details to them
        
  Using standard qt objects it is not always possible to get specific details from the 
  components. This TasHelperInterface implementation will allow us to add details from 
  the wanted dui components that are not accesible through the property or other
  generic way. 
*/

/*!
  Constructor
*/
    UtilFixturePlugin::UtilFixturePlugin(QObject* parent)
        :QObject(parent)
{}

/*!
  Destructor
*/
UtilFixturePlugin::~UtilFixturePlugin()
{}

/*!
  Implementation for traverse so always true.
*/
bool UtilFixturePlugin::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    // place your own code below
    bool result = true;

    if(actionName == "showMessage"){
        QMessageBox msgBox;        
        msgBox.setText(parameters.value("text"));
        msgBox.exec();
    }
    // set the stdOut if you wish to pass information back to MATTI
    else if(actionName == "fail"){
        stdOut = "The execution failed. Parameters were {";
        result =  false;
    }
    else if(actionName == "contextMenu"){
        TasLogger::logger()->debug("UtilFixturePlugin::execute contextMenu");
        QObject* target = 0;
        QPoint globalPoint(-1, -1);
        QPoint localPoint(-1, -1);
        if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget");
            QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
            if (widget) {
                localPoint = widget->rect().center();
                globalPoint = widget->mapToGlobal(localPoint);
                target = qobject_cast<QObject*>(widget);
            }
        }
        else if(parameters.value(OBJECT_TYPE) == GRAPHICS_ITEM_TYPE ){
            TasLogger::logger()->debug("UtilFixturePlugin::execute graphicsitem");
            QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);
            if(item) {
                QWidget* widget = TestabilityUtils::viewPortAndPosition(item, globalPoint);
                localPoint = widget->mapFromGlobal(globalPoint);
                target = qobject_cast<QObject*>(widget);
            }
        }

        if(localPoint.x() > -1 && localPoint.y() > -1) {
            TasLogger::logger()->debug("UtilFixturePlugin::execute local point x:" + QString().setNum(localPoint.x()) + " y:" + QString().setNum(localPoint.y()) + " global point x:" + QString().setNum(globalPoint.x()) + " y:" + QString().setNum(globalPoint.y()));
            //QEvent event = QContextMenuEvent(QContextMenuEvent::Other, point );
            //QEvent event = QContextMenuEvent(QContextMenuEvent::Mouse, localPoint, globalPoint );
            QContextMenuEvent e(QContextMenuEvent::Mouse, localPoint, globalPoint, 0);
            QSpontaneKeyEvent::setSpontaneous(&e);
            qApp->notify(target, &e);
        }
    }
    else if(actionName == "ensureVisible"){
        TasLogger::logger()->debug("UtilFixturePlugin::execute ensureVisible");
        QWidget* widget = 0;
        QGraphicsItem* item = 0;

        if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
            TasLogger::logger()->debug("UtilFixturePlugin::execute ensureVisible widget type");
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget");
            widget = reinterpret_cast<QWidget*>(objectInstance);
        }
        
        if (widget) {
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget != null");
            item = reinterpret_cast<QGraphicsItem*>(widget);
        } else {
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget == null");
            item = reinterpret_cast<QGraphicsItem*>(objectInstance);
        }

        if (item) {
            TasLogger::logger()->debug("UtilFixturePlugin::execute item != null");
            QGraphicsView* view = TestabilityUtils::getViewForItem(item);
            if (view) {
                view->ensureVisible(item);
            }
            else {
                TasLogger::logger()->debug("UtilFixturePlugin::execute view == null");
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
	  //isWidgetRunning
    else if(actionName == "isWidgetRunning"){
    QString key;
    QString value;
        QHash<QString, QString>::const_iterator i = parameters.constBegin();
        while (i != parameters.constEnd()) {

            key=i.key();
            //stdOut.append("The key is" + key);
            value=i.value();
            ++i;
            
            if (key.compare("widgetName")==0){
             break;
            }
        }
      TBool myResult = EFalse;
      myResult = isWidgetRunning(value);
      if (myResult){
        stdOut.append("True");
      }else{
        stdOut.append("False");
      }
    }
    else{
        stdOut = "The execution was ok. Parameters were {";
        result =  true;
    }
//    QHash<QString, QString>::const_iterator i = parameters.constBegin();
//    while (i != parameters.constEnd()) {
//        stdOut.append("(");
//        stdOut.append(i.key());
//        stdOut.append("=>");
//        stdOut.append(i.value());
//        stdOut.append(")");
//        ++i;
//        
//        stdOut.append("}");
//        // set the return value as boolean
//    }
    return result;
}

bool UtilFixturePlugin::isWidgetRunning(QString appId)
{
    TInt error = KErrNone;
    TBool isWidgetRunning = EFalse;
    TPtrC widgetName(static_cast<const TUint16*>(appId.utf16()));
    
    // query registry for UID for widget
    RWidgetRegistryClientSession registryClient;
    error = registryClient.Connect();
    if (error != KErrNone){
      return EFalse;
    }
    TUid widgetUid(KNullUid);
    RWidgetInfoArray widgets;
    TRAP(error, registryClient.InstalledWidgetsL(widgets));
    if (!error) {
        for (TInt i(0); i < widgets.Count(); ++i) {
            CWidgetInfo* info = widgets[i];
    
            // Note that bundleId is pushed onto the cleanup stack
            // and needs a PopAndDestroy() for cleanup later
            TBuf16<128> bundleId;
            //TPt16 ptr(bundleId.Des());
            //registryClient.GetWidgetBundleId(info->iUid, ptr);
            registryClient.GetWidgetBundleId(info->iUid, bundleId);
            if (bundleId.CompareF(widgetName) == 0) {
                isWidgetRunning = registryClient.IsWidgetRunning(info->iUid);
                break;
            }
            //CleanupStack::PopAndDestroy();   // bundleId
        }
    }
    registryClient.Disconnect();
    return isWidgetRunning;
}
