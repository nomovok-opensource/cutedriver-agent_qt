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
#include <QPoint>
#include <QRect>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>

#include "recorderservice.h"
#include "testabilityutils.h"
#include "tastraverserloader.h"
#include "taslogger.h"

/*!
  \class RecorderService
  \brief RecorderService collects events

*/    

RecorderService::RecorderService(QObject* parent)
    :QObject(parent)
{
    mTasModel = new TasDataModel();
    mTasEvents = 0;
    TasTraverserLoader loader;
    mTraversers = loader.loadTraversers();
}

RecorderService::~RecorderService()
{    
    delete mTasModel;
    mTraversers.clear();
}

bool RecorderService::executeService(TasCommandModel& model, TasResponse& response)
{    
    if(model.service() == serviceName() ){
        performRecorderCommands(model, response);
        return true;
    }
    else{
        return false;
    }
}

void RecorderService::performRecorderCommands(TasCommandModel& model, TasResponse& response)
{
    QByteArray* message = 0;
    QListIterator<TasTarget*> i(model.targetList());
    bool commandExecuted = false;
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();        
        if(commandTarget->type() == TYPE_APPLICATION_VIEW){
            TasCommand* command = commandTarget->findCommand("Start");            
            if(command){
                start();
                commandExecuted = true;
                eventCounter = 0;
            }
            command = commandTarget->findCommand("Stop");
            if(command){
                qApp->removeEventFilter(this);
                mTasEvents = 0;
                eventCounter = 0;
                mTasModel->clearModel();
                commandExecuted = true;
            }
            command = commandTarget->findCommand("Print");
            if(command){
                if(mTasEvents != 0){
                    mTasEvents->addAttribute("eventCount", eventCounter);
                    SerializeFilter* filter = new SerializeFilter();		    		
                    filter->serializeDuplicates(true);		    		
                    message = new QByteArray();
                    mTasModel->serializeModel(*message, filter);
                    commandExecuted = true;
                }
                else{
                    message = new QByteArray(QString("Event listening not enabled!").toUtf8());
                }
            }
            break;
        }
    }
    if(commandExecuted){
        if(!message){
            response.setData(OK_MESSAGE);
        }
        else{
            response.setData(message);
        }
    }
    else{
        response.setErrorMessage(PARSE_ERROR);
    }
}


void RecorderService::start()
{
    mTasEvents = 0;
    mTasModel->clearModel();

    TasObjectContainer& container = mTasModel->addNewObjectContainer(1, "QT4.4", "qt");
    mTasEvents = &container.addNewObject(0, "QtRecordedEvents", "events");

    qApp->installEventFilter(this);    
}


/*!
  Filter to receive all events and store events that have been added to the inclusion list
*/
 bool RecorderService::eventFilter(QObject *target, QEvent *event)
 {    
    if (mTasEvents){
        QString eventType = TestabilityUtils::eventType(event) ;

        // check that the event type is in the list of events to be listened or ALL is defined
        if( target->isWidgetType() && (eventType  == "MouseButtonRelease" || eventType == "MouseButtonPress" || eventType == "MouseMove")){

            TasObject& eventObj = mTasEvents->addObject();
            eventObj.setId(QString::number(eventCounter));
            eventObj.setType(QString("event"));
            eventObj.setName(eventType);
            eventObj.addAttribute("timeStamp", QDateTime::currentDateTime().toString(QString("yyyyMMddhhmmsszzz")));

            QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
            eventObj.addAttribute("x", mouseEvent->y());
            eventObj.addAttribute("y", mouseEvent->y());
            eventObj.addAttribute("globalX", mouseEvent->globalX());
            eventObj.addAttribute("globalY", mouseEvent->globalY());
            eventObj.addAttribute("button", mouseEvent->button());

            QPoint position = mouseEvent->pos();
            QWidget* widget = qobject_cast<QWidget*>(target);            
            //add window position
            QPoint windowPoint = widget->mapTo(widget->window(), position);
            eventObj.addAttribute("windowX", windowPoint.x());
            eventObj.addAttribute("windowY", windowPoint.y());

            TasObject& targetObj = eventObj.addObject();
            QWidget* parentWidget = widget->parentWidget();
            if(parentWidget && parentWidget->inherits("QGraphicsView")){
                QGraphicsView* view = qobject_cast<QGraphicsView*>(parentWidget);
                if(view->viewport() == widget && view->scene()){
                    //take the item at the point
                    QGraphicsScene* scene = view->scene();                
                    QGraphicsItem* graphicsItem = scene->itemAt(view->mapToScene(position));
                    if(graphicsItem){                        
                        if (graphicsItem->isWindow() || graphicsItem->isWidget()) {
                            QObject * objectAt = TestabilityUtils::castToGraphicsWidget(graphicsItem);
                            printTargetDetails(objectAt, targetObj);
                        }
                        else{
                            targetObj.setType("QGraphicsItem");
                            QHashIterator<QString, TasTraverseInterface*> i(mTraversers);
                            while (i.hasNext()) {
                                i.next();
                                i.value()->traverseGraphicsItem(&targetObj, graphicsItem);
                            }    
                        }    
                    }
                    //calculate window coordinates                   
                    //windowPoint = view->mapTo(view->window(), position);
                }
                else{
                    printTargetDetails(target, targetObj);
                }
            }
            else{
                printTargetDetails(target, targetObj);
            }
            //set the id to match the event
            //makes it possible to get details from xml tree using the same id
            targetObj.setId(QString::number(eventCounter));
            //increase for the next event
            eventCounter++;
        }
    }
    else {
        TasLogger::logger()->debug("TasCommander::eventfilter : tasEvent was null!!");
    }
    return false;
}

void RecorderService::printTargetDetails(QObject* target, TasObject& targetObj)
{
    QHashIterator<QString, TasTraverseInterface*> i(mTraversers);
    while (i.hasNext()) {
        i.next();
        i.value()->traverseObject(&targetObj, target);
    }
}

