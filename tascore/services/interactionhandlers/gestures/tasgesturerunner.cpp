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

#include "tasgesturerunner.h" 

TasGestureRunner::TasGestureRunner(TasGesture* gesture, QObject* parent)
    :QObject(parent)
{
    mGesture = gesture;
}

TasGestureRunner::~TasGestureRunner()
{
    delete mGesture;
}

void TasGestureRunner::timerEvent(qreal);
{
    if(gesture->isMultiTouch() || gesture->pointerType() == MouseHandler::TypeTouch
    doTouchUpdate(mTargetDetails.widget, mTargetDetails.item, mGesture->pointsAt(value));
}
void TasGestureRunner::finished();
void TasGestureRunner::releaseMouse();
void TasGestureRunner::startGesture();

bool TasGestureRunner::eventFilter(QObject *target, QEvent *event)
{
    //for some reason the following event is sent 
    //and it will cause a mouse ungrab events to be propagated
    //which will lead to all future events to be ignored
    //so eat the event..
    if( event->type() == QEvent::MouseMove){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == 0 && mouseEvent->buttons() == 0) {
            return true;
        }
        return false;
    }
    else{
        return false;
    }
}
