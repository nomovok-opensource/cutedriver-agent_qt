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
#include <QDesktopWidget>

#include "tascoreutils.h"
#include "tastoucheventgenerator.h"
#include "taslogger.h"

int TasTouchEventGenerator::mTouchPointCounter = 0;

TasTouchEventGenerator::TasTouchEventGenerator(QObject* parent)
    :QObject(parent)
{
}

TasTouchEventGenerator::~TasTouchEventGenerator()
{
}

void TasTouchEventGenerator::doTouchBegin(QWidget* target, QGraphicsItem* targetItem, 
                                          QPoint point, bool primary, QString extraIdentifier)
{
    doTouchBegin(target, targetItem, toTouchPoints(point,primary), extraIdentifier);
}

void TasTouchEventGenerator::doTouchUpdate(QWidget* target, QGraphicsItem* targetItem, QPoint point, bool primary)
{
    doTouchUpdate(target, targetItem, toTouchPoints(point,primary));
}

void TasTouchEventGenerator::doTouchEnd(QWidget* target, QGraphicsItem* targetItem, QPoint point, bool primary, 
                                        QString extraIdentifier)
{
    doTouchEnd(target, targetItem, toTouchPoints(point,primary), extraIdentifier);
}


void TasTouchEventGenerator::doTouchBegin(QWidget* target, QGraphicsItem* targetItem, 
                                          QList<TasTouchPoints> points, QString extraIdentifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, targetItem, Qt::TouchPointPressed, points, extraIdentifier);
    QTouchEvent* touchPress = new QTouchEvent(QEvent::TouchBegin, QTouchEvent::TouchScreen, Qt::NoModifier, Qt::TouchPointPressed, touchPoints);
    touchPress->setWidget(target);
    sendTouchEvent(target, touchPress);
}

void TasTouchEventGenerator::doTouchUpdate(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, targetItem, Qt::TouchPointMoved, points);
    QTouchEvent* touchMove = new QTouchEvent(QEvent::TouchUpdate, QTouchEvent::TouchScreen, Qt::NoModifier, Qt::TouchPointMoved, touchPoints);
    touchMove->setWidget(target);
    sendTouchEvent(target, touchMove);
}

void TasTouchEventGenerator::doTouchEnd(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points, 
                                        QString extraIdentifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, targetItem, Qt::TouchPointReleased, points, extraIdentifier);
    QTouchEvent *touchRelease = new QTouchEvent(QEvent::TouchEnd, QTouchEvent::TouchScreen, Qt::NoModifier, Qt::TouchPointReleased, touchPoints);
    touchRelease->setWidget(target);
    sendTouchEvent(target, touchRelease);
}


void TasTouchEventGenerator::sendTouchEvent(QWidget* target, QTouchEvent* event)
{
    QSpontaneKeyEvent::setSpontaneous(event);
    qApp->postEvent(target, event);   
    qApp->processEvents();
}

QList<QTouchEvent::TouchPoint> TasTouchEventGenerator::convertToTouchPoints(TargetData targetData, Qt::TouchPointState state)
{
    return convertToTouchPoints(targetData.target,targetData.targetItem, state, toTouchPoints(targetData.targetPoint, false));
}

QList<QTouchEvent::TouchPoint> TasTouchEventGenerator::convertToTouchPoints(QWidget* target, QGraphicsItem* targetItem, Qt::TouchPointState state,
                                                                            QList<TasTouchPoints> points, QString extraIdentifier)
{
    bool store = true;
    QList<QVariant> pointIds;
    QString itemId;
    //we need to store the touchpoint ids, the same id must be attached for untill touch point released
    if(targetItem) {
        itemId = TasCoreUtils::pointerId(targetItem);        
        //extraIdentifier needed to differentiate taps that occur in the same item but different coordinates
        //in normal object taps the identifier is empty
        itemId.append(extraIdentifier);        
        QVariant pointStore = qApp->property(itemId.toAscii());
        if(pointStore.isValid()){
            pointIds = pointStore.toList();
        }
        if(state == Qt::TouchPointReleased){
            //set invalid to remove the list
            qApp->setProperty(itemId.toAscii(), QVariant());
            store = false;
        }       
    }
    else{
        store = false;                        
    }
    QList<QTouchEvent::TouchPoint> touchPoints;    
    if(!points.isEmpty()){
        for(int i = 0 ; i < points.size() ; i++){
            if(pointIds.size() <= i ){
                mTouchPointCounter++;
                pointIds.append(QVariant(mTouchPointCounter));                
            }
            touchPoints.append(makeTouchPoint(target, points.at(i), state, pointIds.at(i).toInt()));
        }
    }
    if(store && !itemId.isEmpty()){
        qApp->setProperty(itemId.toAscii(), QVariant(pointIds));
    }

    return touchPoints;
}

QTouchEvent::TouchPoint TasTouchEventGenerator::makeTouchPoint(QWidget* target, TasTouchPoints points,
                                                               Qt::TouchPointState state, int id)
{
    QTouchEvent::TouchPoint touchPoint(id);
    Qt::TouchPointStates states = state;
    if(points.isPrimary){
        states |= Qt::TouchPointPrimary;
    }
    touchPoint.setState(states);
    touchPoint.setPos(target->mapFromGlobal(points.screenPoint));    
    touchPoint.setScreenPos(points.screenPoint);    
    QRect screenGeometry = QApplication::desktop()->screenGeometry(points.screenPoint);
    touchPoint.setNormalizedPos(QPointF(points.screenPoint.x() / screenGeometry.width(),
                                        points.screenPoint.y() / screenGeometry.height()));

    //in addition to the position we also need to set last and start positions as 
    //some gesture may depend on them
    if(!points.lastScreenPoint.isNull()){
        touchPoint.setLastPos(target->mapFromGlobal(points.lastScreenPoint));    
        touchPoint.setLastScreenPos(points.lastScreenPoint);    
        touchPoint.setLastNormalizedPos(QPointF(points.lastScreenPoint.x() / screenGeometry.width(),
                                                points.lastScreenPoint.y() / screenGeometry.height()));
    }
    if(!points.startScreenPoint.isNull()){
        touchPoint.setStartPos(target->mapFromGlobal(points.startScreenPoint));    
        touchPoint.setStartScreenPos(points.startScreenPoint);    
        touchPoint.setStartNormalizedPos(QPointF(points.startScreenPoint.x() / screenGeometry.width(),
                                                points.startScreenPoint.y() / screenGeometry.height()));
    }
    return touchPoint;
}

QList<TasTouchPoints> TasTouchEventGenerator::toTouchPoints(QPoint point, bool primary)
{
    QList<TasTouchPoints> points;
    TasTouchPoints touchPoint;
    touchPoint.screenPoint = point;
    touchPoint.isPrimary = primary;
    points.append(touchPoint);
    return points;
}


