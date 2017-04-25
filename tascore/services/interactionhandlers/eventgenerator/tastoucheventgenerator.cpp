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

#include <QWindow>
#include <QScreen>

#include "tascoreutils.h"
#include "tastoucheventgenerator.h"
#include "taslogger.h"
#include "mousehandler.h"

int TasTouchEventGenerator::mTouchPointCounter = 0;

TasTouchEventGenerator::TasTouchEventGenerator(QObject* parent)
    :QObject(parent), mTouchDevice(NULL)
{
}

TasTouchEventGenerator::~TasTouchEventGenerator()
{
    if (mTouchDevice) {
        delete mTouchDevice;
        mTouchDevice = NULL;
    }
}

void TasTouchEventGenerator::doTouchBegin(const TasEventTarget& target, QPoint point, QString identifier)
{
    doTouchBegin(target, toTouchPoints(point), identifier);
}

void TasTouchEventGenerator::doTouchUpdate(const TasEventTarget& target, QPoint point, QString identifier)
{
    doTouchUpdate(target, toTouchPoints(point), identifier);
}

void TasTouchEventGenerator::doTouchEnd(const TasEventTarget& target, QPoint point, QString identifier)
{
    doTouchEnd(target, toTouchPoints(point), identifier);
}


void TasTouchEventGenerator::doTouchBegin(const TasEventTarget& target, QList<TasTouchPoints> points, QString identifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, Qt::TouchPointPressed, points, identifier);

    //Create QTouchDevice if it is missing
    if (!mTouchDevice) {
        createTouchDevice();
    }

    QTouchEvent* touchPress = new QTouchEvent(QEvent::TouchBegin, mTouchDevice, Qt::NoModifier, Qt::TouchPointPressed, touchPoints);
    touchPress->setTarget(target.receiver());
    sendTouchEvent(target, touchPress);
}

void TasTouchEventGenerator::doTouchUpdate(const TasEventTarget& target, QList<TasTouchPoints> points, QString identifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, Qt::TouchPointMoved, points, identifier);

    //Create QTouchDevice if it is missing
    if (!mTouchDevice) {
        createTouchDevice();
    }

    QTouchEvent* touchMove = new QTouchEvent(QEvent::TouchUpdate, mTouchDevice, Qt::NoModifier, Qt::TouchPointMoved, touchPoints);
    touchMove->setTarget(target.receiver());
    sendTouchEvent(target, touchMove);
}

void TasTouchEventGenerator::doTouchEnd(const TasEventTarget& target, QList<TasTouchPoints> points, QString identifier)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, Qt::TouchPointReleased, points, identifier);

    //Create QTouchDevice if it is missing
    if (!mTouchDevice) {
        createTouchDevice();
    }

    QTouchEvent *touchRelease = new QTouchEvent(QEvent::TouchEnd, mTouchDevice, Qt::NoModifier, Qt::TouchPointReleased, touchPoints);
    touchRelease->setTarget(target.receiver());
    sendTouchEvent(target, touchRelease);
}


void TasTouchEventGenerator::sendTouchEvent(const TasEventTarget& target, QTouchEvent* event)
{
    QSpontaneKeyEvent::setSpontaneous(event);
    qApp->postEvent(target.receiver(), event);
    qApp->processEvents();
}

QList<QTouchEvent::TouchPoint> TasTouchEventGenerator::convertToTouchPoints(TargetData targetData, Qt::TouchPointState state)
{
    return convertToTouchPoints(TasEventTarget(targetData), state, toTouchPoints(targetData.targetPoint),
                                TasCoreUtils::pointerId(targetData.targetItem));
}

QList<QTouchEvent::TouchPoint> TasTouchEventGenerator::convertToTouchPoints(const TasEventTarget& target, Qt::TouchPointState state,
                                                                            QList<TasTouchPoints> points,
                                                                            QString identifier)
{
    QList<QVariant> pointIds;
    if(!identifier.isEmpty()) {
        QVariant pointStore = qApp->property(identifier.toLatin1());
        if(pointStore.isValid()){
            pointIds = pointStore.toList();
        }
    }

    QList<QTouchEvent::TouchPoint> touchPoints;
    if(!points.isEmpty()){
        for(int i = 0 ; i < points.size() ; i++){
            if(pointIds.size() <= i ){
                mTouchPointCounter++;
                pointIds.append(QVariant(mTouchPointCounter));
            }
            touchPoints.append(makeTouchPoint(target, points.at(i), state, pointIds.at(i).toInt()-1));
        }
    }


    if(state == Qt::TouchPointReleased){
        qApp->setProperty(identifier.toLatin1(), QVariant());
        mTouchPointCounter = 0;
    }
    else if(!identifier.isEmpty()){
        //we store the point id to the app as property
        //this allows new gestures to use the ids when needed
        qApp->setProperty(identifier.toLatin1(), QVariant(pointIds));
    }
    return touchPoints;
}

QTouchEvent::TouchPoint TasTouchEventGenerator::makeTouchPoint(const TasEventTarget& target, TasTouchPoints points,
                                                               Qt::TouchPointState state, int id)
{
    TasLogger::logger()->debug("TasTouchEventGenerator:: generating point with id: " +
                               QString::number(id));
    QTouchEvent::TouchPoint touchPoint(id);
    Qt::TouchPointStates states = state;
    touchPoint.setPressure(1.0);
    touchPoint.setState(states);
    touchPoint.setPos(target.mapFromGlobal(points.screenPoint));

    QWidget* parentWidget = target.parentWidget();
    if(parentWidget && parentWidget->inherits("QGraphicsView")){
        QGraphicsView* view = qobject_cast<QGraphicsView*>(parentWidget);
        if (view) {
            touchPoint.setScenePos(view->mapToScene(points.screenPoint));
        }
    } else {
        touchPoint.setScenePos(points.screenPoint);
    }

    touchPoint.setScreenPos(points.screenPoint);

    //in addition to the position we also need to set last and start positions as
    //some gesture may depend on them
    if(!points.lastScreenPoint.isNull()){
        touchPoint.setLastPos(target.mapFromGlobal(points.lastScreenPoint));
        touchPoint.setLastScreenPos(points.lastScreenPoint);
    }
    if(!points.startScreenPoint.isNull()){
        touchPoint.setStartPos(target.mapFromGlobal(points.startScreenPoint));
        touchPoint.setStartScreenPos(points.startScreenPoint);
    }
    return touchPoint;
}

QList<TasTouchPoints> TasTouchEventGenerator::toTouchPoints(QPoint point)
{
    QList<TasTouchPoints> points;
    points.append(toTouchPoint(point));
    return points;
}
TasTouchPoints TasTouchEventGenerator::toTouchPoint(QPoint point)
{
    TasTouchPoints touchPoint;
    touchPoint.screenPoint = point;
    return touchPoint;
}

bool TasTouchEventGenerator::areIdentical(QList<TasTouchPoints> points1, QList<TasTouchPoints> points2)
{
    if(points1.size() != points2.size()){
        return false;
    }
    //loop points to detect differences
    for(int i = 0 ; i < points1.size() ; i++){
        TasTouchPoints t = points1.at(i);
        TasTouchPoints p = points2.at(i);
        if(p.screenPoint != t.screenPoint || t.lastScreenPoint != p.lastScreenPoint ||
           p.startScreenPoint != t.startScreenPoint){
            return false;
        }
    }
    return true;
}

void TasTouchEventGenerator::createTouchDevice()
{
    mTouchDevice = new QTouchDevice();
    mTouchDevice->setType(QTouchDevice::TouchScreen);
    mTouchDevice->setName(metaObject()->className());
}
