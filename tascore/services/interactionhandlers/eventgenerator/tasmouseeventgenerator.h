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
 


#ifndef TASMOUSEEVENTGENERATOR_H
#define TASMOUSEEVENTGENERATOR_H

#include <QApplication>
#include <QPoint>
#include <QWidget>
#include <QMouseEvent>
#include <QCursor>

#include "taseventtarget.h"

class TasMouseEventGenerator : public QObject
{
   Q_OBJECT
public:
    TasMouseEventGenerator(QObject* parent=0);
    ~TasMouseEventGenerator();

    void setUseTapScreen(bool use);

public slots:	
    void doMousePress(const TasEventTarget& target, Qt::MouseButton button, QPoint point, uint pointerNumber=0);
    void doMouseRelease(const TasEventTarget& target, Qt::MouseButton button, QPoint point, uint pointerNumber=0);
    void doMouseMove(const TasEventTarget& target, QPoint point, Qt::MouseButton button=Qt::NoButton, uint pointerNumber=0);
    void doScroll(const TasEventTarget& target, QPoint& point, int delta, Qt::MouseButton button,  Qt::Orientation orient);
    void doMouseDblClick(const TasEventTarget& target, Qt::MouseButton button, QPoint point);
    void moveCursor(QPoint point, uint pointerNumber=0);

private:
    void sendMouseEvent(const TasEventTarget& target, QMouseEvent* event, uint pointerNumber=0);


private:
    bool mUseTapScreen;
};

#endif
