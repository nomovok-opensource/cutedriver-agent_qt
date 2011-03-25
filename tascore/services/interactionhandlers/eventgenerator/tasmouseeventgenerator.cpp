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

void TasMouseEventGenerator::doMousePress(QWidget* target, Qt::MouseButton button, QPoint point, uint pointerNumber)
{
    QMouseEvent* eventPress = new QMouseEvent(QEvent::MouseButtonPress, target->mapFromGlobal(point), point, button, button, 0);    
    sendMouseEvent(target, eventPress, pointerNumber);
}
void TasMouseEventGenerator::doMouseRelease(QWidget* target, Qt::MouseButton button, QPoint point, uint pointerNumber)
{
    QMouseEvent* eventRelease = new QMouseEvent(QEvent::MouseButtonRelease, target->mapFromGlobal(point), point, button, Qt::NoButton, 0);
    sendMouseEvent(target, eventRelease, pointerNumber);
}
void TasMouseEventGenerator::doMouseMove(QWidget* target, QPoint point, Qt::MouseButton button, uint pointerNumber )
{
    moveCursor(point);
    QMouseEvent* eventMove = new QMouseEvent(QEvent::MouseMove, target->mapFromGlobal(point), point, button, button, 0);
    sendMouseEvent(target, eventMove, pointerNumber);
}
void TasMouseEventGenerator::doScroll(QWidget* target, QPoint& point, int delta, Qt::MouseButton button,  Qt::Orientation orient)
{
    QWheelEvent* event = new QWheelEvent(point, target->mapToGlobal(point), delta, button, 0, orient);
    qApp->postEvent(target, event);
}

void TasMouseEventGenerator::doMouseDblClick(QWidget* target, Qt::MouseButton button, QPoint point)
{
    QMouseEvent* eventDblClick = new QMouseEvent(QEvent::MouseButtonDblClick, target->mapFromGlobal(point), point, button, Qt::NoButton, 0);    
    sendMouseEvent(target, eventDblClick);
}
void TasMouseEventGenerator::sendMouseEvent(QWidget* target, QMouseEvent* event, uint pointerNumber)
{
    if(mUseTapScreen){
        TasDeviceUtils::sendMouseEvent(event->globalX(), event->globalY(), event->button(), event->type(), pointerNumber);
    } else {
        QSpontaneKeyEvent::setSpontaneous(event);
        qApp->postEvent(target, event);
        qApp->processEvents();
    }
}

void TasMouseEventGenerator::moveCursor(QPoint point, uint pointerNumber)
{
    if (mUseTapScreen) {
        //TasDeviceUtils::sendMouseEvent(point.x(), point.y(), Qt::NoButton, QEvent::MouseMove, pointerNumber);
    } else {
        QCursor::setPos(point);
    }
}

