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

#include "eventservice.h"
#include "testabilityutils.h"
#include "taslogger.h"


/*!
  \class EventService
  \brief EventService collects events

*/    

EventService::EventService()
{
}

EventService::~EventService()
{
    qDeleteAll(mEventFilters);
    mEventFilters.clear();
}

bool EventService::executeService(TasCommandModel& model, TasResponse& response)
{    
    if(model.service() == serviceName() ){
        performEventCommands(model, response);
        return true;
    }
    else{
        return false;
    }
}

void EventService::performEventCommands(TasCommandModel& model, TasResponse& response)
{
    QString errorMessage;
    QByteArray message;
    QListIterator<TasTarget*> i(model.targetList());
    bool commandExecuted = false;
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();
        TasCommand* command = commandTarget->findCommand("EnableEvents");            
        if(command){
            TasEventFilter* filter = getFilterForTarget(commandTarget, true);
            if(filter){
                QString eventsStr = command->parameter("EventsToListen");   
                filter->startFiltering(eventsStr.split(",", QString::SkipEmptyParts));
                response.setData(QString(OK_MESSAGE));
            }
            else{
                response.setErrorMessage("Target could not be found.");
            }
            commandExecuted = true;
        }
        command = commandTarget->findCommand("DisableEvents");
        if(command){
            TasEventFilter* filter = getFilterForTarget(commandTarget, false);
            if(filter){
                mEventFilters.remove(commandTarget->id());
                delete filter;
                response.setData(QString(OK_MESSAGE));
            }
            else{
                response.setErrorMessage("No filter set for object.");
            }
            commandExecuted = true;
        }
        command = commandTarget->findCommand("GetEvents");
        if(command){
            TasEventFilter* filter = getFilterForTarget(commandTarget, false);
            if(filter){
                response.setData(filter->getEvents());
            }
            else{
                response.setErrorMessage("Event listening not enabled!");
            }
            commandExecuted = true;                
        }
        break;        
    }

    if(!commandExecuted){        
        response.setErrorMessage(PARSE_ERROR);       
    }
}

void EventService::enableEvents(QString targetId, QObject* target, QStringList eventsToListen)
{    
    TasEventFilter* filter = 0;
    if(mEventFilters.contains(targetId)){
        filter = mEventFilters.value(targetId);
    }
    else{
        filter = new TasEventFilter(target);
        mEventFilters.insert(targetId, filter);
    }
    filter->startFiltering(eventsToListen);
}

/*!
  Looks for a filter for the target. Returns null of target does not specify the target or no
  target is found for the id and create is false. Will create a new filter is create is true and
  no existing filter found.
 */
TasEventFilter* EventService::getFilterForTarget(TasTarget* commandTarget, bool create)
{
    TasEventFilter* filter = 0;
    QString targetId = commandTarget->id();
    QString targetType = commandTarget->type();
    //are required for command completion
    if(targetId.isEmpty() || targetType.isEmpty()){
        return 0;
    }
    if(targetType == TYPE_APPLICATION_VIEW){
        targetId = QString::number(qApp->applicationPid());
    }

    if(mEventFilters.contains(targetId)){
        filter = mEventFilters.value(targetId);
    }
    else if(create){
        QObject* target = 0;
        if(targetType == TYPE_GRAPHICS_VIEW){
            QGraphicsItem* item = findGraphicsItem(targetId); 
            target = TestabilityUtils::castToGraphicsWidget(item);
        }
        else if(targetType == TYPE_STANDARD_VIEW){
            target = findWidget(targetId);
        }        
        else if(targetType == TYPE_APPLICATION_VIEW){
            target= qApp;
        }    
        if(target){
            filter = new TasEventFilter(target);
            mEventFilters.insert(targetId, filter);
        }
    }
    else{
        filter = 0;
    }
    return filter;
}

/*!
  Adds a processstart event to model. Event collecting must be enabled if not this
  function will do nothing.
 */
void EventService::addProcessStartEvent(QDateTime startTime)
{
    QString id = QString::number(qApp->applicationPid());
    if(mEventFilters.contains(id)){
        TasEventFilter* appFilter = mEventFilters.value(id);
        appFilter->addStartTime(startTime);
    }
}


TasEventFilter::TasEventFilter(QObject* target, QObject* parent)
    :QObject(parent)
{    
    mTarget = target;
    mTasModel = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = mTasModel->addNewObjectContainer(1, qtVersion, "qt");
    mTasEvents = &container.addNewObject(0, "QtApplicationEvents", "events");   
}

TasEventFilter::~TasEventFilter()
{
    mTarget->removeEventFilter(this);
    mTarget = 0;
    delete mTasModel;
}

void TasEventFilter::startFiltering(QStringList eventsToListen)
{
    mEventsToListen = eventsToListen;
    mTarget->installEventFilter(this);    
}

void TasEventFilter::addStartTime(QDateTime startTime)
{
    if(mTasEvents){
        TasObject& eventObj = mTasEvents->addObject();
        eventObj.setId(TasCoreUtils::pointerId(&startTime));
        eventObj.setType(QString("event"));
        eventObj.setName(PROCESS_START_TIME);
        eventObj.addAttribute("timeStamp", startTime.toString(DATE_FORMAT));
    }    
}

bool TasEventFilter::eventFilter(QObject *target, QEvent *event)
{
    //    TasLogger::logger()->debug("TasEventFilter::eventFilter");
    if (mTasEvents){
        QString eventType = TestabilityUtils::eventType(event) ;

        // check that the event type is in the list of events to be listened or ALL is defined
        if (!mEventsToListen.contains(eventType) && !mEventsToListen.contains(QString("ALL")))
            return false;

        TasObject& eventObj = mTasEvents->addObject();
        eventObj.setId(TasCoreUtils::pointerId(event));                                     
        eventObj.setType(QString("event"));
        eventObj.setName(eventType);
        eventObj.addAttribute("timeStamp", QDateTime::currentDateTime().toString(DATE_FORMAT));
        addMouseEventDetails(event, eventObj);

        if(target){
            TasObject& targetObj = eventObj.addObject();
            targetObj.setId(TasCoreUtils::objectId(target));
            targetObj.setName(target->objectName() != NULL ? target->objectName() : "NoName");
            targetObj.setType(target->metaObject()->className());

            if(target->metaObject()){
                const QMetaObject *metaobject = target->metaObject();
                int count = metaobject->propertyCount();
                for (int i=0; i<count; i++){
                    QMetaProperty metaproperty = metaobject->property(i);
                    const char *name = metaproperty.name();
                    QVariant value = target->property(name);
                    if(value.isValid()){
                        targetObj.addAttribute(name, value.toString());
                    }
                }
            }
        }
    }
    else {
        TasLogger::logger()->error("TasEventFilter::eventFilter : tasEvent was null!!");
    }
    return false;
}

void TasEventFilter::addMouseEventDetails(QEvent *event, TasObject& eventObj)
{
    QEvent::Type type = event->type();
    if(type == QEvent::MouseButtonDblClick ||  type == QEvent::MouseButtonPress ||
       type == QEvent::MouseButtonRelease || type == QEvent::MouseMove){
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        eventObj.addAttribute("globalX", mouseEvent->globalX()).setType("int");
        eventObj.addAttribute("globalY", mouseEvent->globalY()).setType("int");
        eventObj.addAttribute("x", mouseEvent->x()).setType("int");
        eventObj.addAttribute("y", mouseEvent->y()).setType("int");
        eventObj.addAttribute("button", mouseEvent->button()).setType("Qt::MouseButton");
    }
    else if(type == QEvent::GraphicsSceneMouseDoubleClick || type == QEvent::GraphicsSceneMouseMove || 
            type == QEvent::GraphicsSceneMousePress || type == QEvent::GraphicsSceneMouseRelease ){
        QGraphicsSceneMouseEvent* mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        eventObj.addAttribute("position", mouseEvent->pos()).setType("QPointF");
        eventObj.addAttribute("scenePosition", mouseEvent->scenePos()).setType("QPointF");
        eventObj.addAttribute("screenPosition", mouseEvent->screenPos()).setType("QPoint");
        eventObj.addAttribute("lastPosition", mouseEvent->lastPos()).setType("QPointF");
        eventObj.addAttribute("lastScenePosition", mouseEvent->lastScenePos()).setType("QPointF");
        eventObj.addAttribute("lastScreenPosition", mouseEvent->lastScreenPos()).setType("QPoint");
        eventObj.addAttribute("button", mouseEvent->button()).setType("Qt::MouseButton");
        if(mouseEvent->button() != Qt::NoButton){
            eventObj.addAttribute("buttonDownPos", mouseEvent->buttonDownPos(mouseEvent->button())).setType("QPointF");
            eventObj.addAttribute("buttonDownScenePos", mouseEvent->buttonDownScenePos(mouseEvent->button())).setType("QPointF");
            eventObj.addAttribute("buttonDownScreenPos", mouseEvent->buttonDownScreenPos(mouseEvent->button())).setType("QPoint");
        }
    }
}


QByteArray TasEventFilter::getEvents()
{
    QByteArray message;
    if(mTasEvents != 0){
        SerializeFilter* filter = new SerializeFilter();		    		
        filter->serializeDuplicates(true);		    		
        mTasModel->serializeModel(message, filter);
    }
    else{
        message = QByteArray(QString("Event listening not enabled!").toUtf8());
    }
    return message;
}
