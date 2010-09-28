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
    mUseTapScreen = use;
}

void TasMouseEventGenerator::doMousePress(QWidget* target, Qt::MouseButton button, QPoint point)
{
    QMouseEvent* eventPress = new QMouseEvent(QEvent::MouseButtonPress, target->mapFromGlobal(point), point, button, button, 0);    
    sendMouseEvent(target, eventPress);
}
void TasMouseEventGenerator::doMouseRelease(QWidget* target, Qt::MouseButton button, QPoint point)
{
    QMouseEvent* eventRelease = new QMouseEvent(QEvent::MouseButtonRelease, target->mapFromGlobal(point), point, button, Qt::NoButton, 0);    
    sendMouseEvent(target, eventRelease);
}
void TasMouseEventGenerator::doMouseMove(QWidget* target, QPoint point, Qt::MouseButton button)
{
    moveCursor(point);
    QMouseEvent* eventMove = new QMouseEvent(QEvent::MouseMove, target->mapFromGlobal(point), point, button, button, 0);
    sendMouseEvent(target, eventMove);
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
void TasMouseEventGenerator::sendMouseEvent(QWidget* target, QMouseEvent* event)
{
    if(mUseTapScreen){
        TasDeviceUtils::sendMouseEvent(event->globalX(), event->globalY(), event->button(), event->type());
    } else {
        QSpontaneKeyEvent::setSpontaneous(event);
        qApp->postEvent(target, event);   
    //in windows this causes unwanted beheviour in other vice versa    
#if (!defined(Q_OS_WIN32) && !defined(Q_OS_WINCE))
        //qApp->processEvents();
        qApp->sendPostedEvents(target, event->type());
#endif
    }
}

void TasMouseEventGenerator::moveCursor(QPoint point)
{
    if (mUseTapScreen) {
        TasDeviceUtils::sendMouseEvent(point.x(), point.y(), Qt::NoButton, QEvent::MouseMove);                
    } else {
        QCursor::setPos(point);
    }
}

