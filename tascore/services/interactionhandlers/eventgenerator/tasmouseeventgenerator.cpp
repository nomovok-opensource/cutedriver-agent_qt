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

#include "tasmouseeventgenerator.h" 
#include "tasdeviceutils.h"
#include "taslogger.h"

#if defined(Q_OS_WIN32)
#include "windows.h"
#endif

TasMouseEventGenerator::TasMouseEventGenerator(QObject* parent)
    :QObject(parent)
{
    mUseTapScreen = false;
}

TasMouseEventGenerator::~TasMouseEventGenerator()
{}

void TasMouseEventGenerator::setUseTapScreen(bool use)
{
    //TasLogger::logger()->debug(QString(__FUNCTION__) + QString::number(use));
    mUseTapScreen = use;
}

void TasMouseEventGenerator::doMousePress(const TasEventTarget& target, Qt::MouseButton button, QPoint point, uint pointerNumber)
{
    QMouseEvent* eventPress = new QMouseEvent(QEvent::MouseButtonPress, target.mapFromGlobal(point), point, button, button, 0);
    sendMouseEvent(target, eventPress, pointerNumber);
}
void TasMouseEventGenerator::doMouseRelease(const TasEventTarget& target, Qt::MouseButton button, QPoint point, uint pointerNumber)
{
    QMouseEvent* eventRelease = new QMouseEvent(QEvent::MouseButtonRelease, target.mapFromGlobal(point), point, button, Qt::NoButton, 0);
    sendMouseEvent(target, eventRelease, pointerNumber);
}
void TasMouseEventGenerator::doMouseMove(const TasEventTarget& target, QPoint point, Qt::MouseButton button, uint pointerNumber )
{
    moveCursor(point);
    QMouseEvent* eventMove = new QMouseEvent(QEvent::MouseMove, target.mapFromGlobal(point), point, button, button, 0);
    sendMouseEvent(target, eventMove, pointerNumber);
}
void TasMouseEventGenerator::doScroll(const TasEventTarget& target, QPoint& point, int delta, Qt::MouseButton button,  Qt::Orientation orient)
{
    QWheelEvent* event = new QWheelEvent(point, target.mapToGlobal(point), delta, button, 0, orient);
    qApp->postEvent(target.receiver(), event);
}

void TasMouseEventGenerator::doMouseDblClick(const TasEventTarget& target, Qt::MouseButton button, QPoint point)
{
    QMouseEvent* eventDblClick = new QMouseEvent(QEvent::MouseButtonDblClick, target.mapFromGlobal(point), point, button, Qt::NoButton, 0);
    sendMouseEvent(target, eventDblClick);
}
void TasMouseEventGenerator::sendMouseEvent(const TasEventTarget& target, QMouseEvent* event, uint pointerNumber)
{
    if(mUseTapScreen){
#if defined(Q_OS_WIN32)
        if( GetForegroundWindow() != target.window()->winId()){
            TasLogger::logger()->debug("TasMouseEventGenerator::sendMouseEvent set foreground");
            SetForegroundWindow(target.window()->winId());
        }
        if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::GraphicsSceneMousePress){
            TasDeviceUtils::sendMouseEvent(event->globalX(), event->globalY(), event->button(), QEvent::MouseMove, pointerNumber);
        }
#endif
        TasDeviceUtils::sendMouseEvent(event->globalX(), event->globalY(), event->button(), event->type(), pointerNumber);
        qApp->processEvents();
        TasLogger::logger()->debug("TasMouseEventGenerator::sendMouseEvent done");
    } else {
        QSpontaneKeyEvent::setSpontaneous(event);
        qApp->postEvent(target.receiver(), event);
        qApp->processEvents();
    }
}

void TasMouseEventGenerator::moveCursor(QPoint point, uint /*pointerNumber*/)
{
    if (mUseTapScreen) {
        //TasDeviceUtils::sendMouseEvent(point.x(), point.y(), Qt::NoButton, QEvent::MouseMove, pointerNumber);
    } else {
        QCursor::setPos(point);
    }
}

