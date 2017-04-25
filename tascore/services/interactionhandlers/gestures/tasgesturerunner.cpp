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

#include "tasgesturerunner.h"
#include "taslogger.h"

TasGestureRunner::TasGestureRunner(TasGesture* gesture, QObject* parent)
    :QObject(parent)
{
    mGesture = gesture;
    connect(&mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(gestureTimerEvent(qreal)));
    connect(&mTimeLine, SIGNAL(finished()), this, SLOT(finished()));
    qApp->installEventFilter(this);
    mMouseGen.setUseTapScreen(gesture->getUseTapScreen());
    startGesture();
}

TasGestureRunner::~TasGestureRunner()
{
    delete mGesture;
}

void TasGestureRunner::startGesture()
{
    mPreviousPoints = mGesture->startPoints();
    int duration = mGesture->getDuration();
    mTimeLine.setDuration(duration);
    mTimeLine.setFrameRange(0,duration/FRAME_RANGE_DIV);

    if(mGesture->isPress()){
        if(!mGesture->getUseTapScreen()){
            if(mGesture->pointerType() == MouseHandler::TypeTouch || mGesture->pointerType() == MouseHandler::TypeBoth){
                mTouchGen.doTouchBegin(TasEventTarget(mGesture), mGesture->startPoints(), mGesture->touchPointIdKey());
            }
            if(mGesture->pointerType() == MouseHandler::TypeMouse || mGesture->pointerType() == MouseHandler::TypeBoth){
                mMouseGen.doMousePress(TasEventTarget(mGesture), mGesture->getMouseButton(), mGesture->startPoints().first().screenPoint);
            }
        }
        else{
            for(int n=0;n<mGesture->startPoints().size();n++){
                mMouseGen.doMousePress(TasEventTarget(mGesture),  mGesture->getMouseButton(), mGesture->startPoints().at(n).screenPoint, n);
            }
        }
    }

    mTimeLine.start();
}

void TasGestureRunner::gestureTimerEvent(qreal value)
{
    move(mGesture->pointsAt(value));
    mPreviousPoints = mGesture->pointsAt(value);
}
void TasGestureRunner::finished()
{
    //    TasLogger::logger()->debug("TasGestureRunner::finished");
    move(mGesture->endPoints());
    if(mGesture->isRelease()){
        if(mGesture->isDrag()){
            //pause for a moment
            QTimer::singleShot(50, this, SLOT(releaseMouse()));
        }
        else{
            releaseMouse();
        }
    }
    else{
        qApp->removeEventFilter(this);
        deleteLater();
    }
}

void TasGestureRunner::releaseMouse()
{

    if(!mGesture->getUseTapScreen()){
        if(mGesture->pointerType() == MouseHandler::TypeTouch || mGesture->pointerType() == MouseHandler::TypeBoth){
            mTouchGen.doTouchEnd(TasEventTarget(mGesture), mGesture->endPoints(), mGesture->touchPointIdKey());
        }
        if(mGesture->pointerType() == MouseHandler::TypeMouse || mGesture->pointerType() == MouseHandler::TypeBoth){
            mMouseGen.doMouseRelease(TasEventTarget(mGesture), mGesture->getMouseButton(), mGesture->endPoints().first().screenPoint);
        }
    }
    else{
        for(int n=0;n<mGesture->endPoints().size();n++){
            mMouseGen.doMouseRelease(TasEventTarget(mGesture),mGesture->getMouseButton(),mGesture->endPoints().at(n).screenPoint, n);
        }
    }

    qApp->removeEventFilter(this);
    deleteLater();
}

void TasGestureRunner::move(QList<TasTouchPoints> points, bool force)
{
    if(!force){
        //check that the point is not the same as before
        //which could cause a long tap instead of a gesture
        if(noMovement(points)){
            return;
        }
    }

    if(!mGesture->getUseTapScreen()){
        if(mGesture->pointerType() == MouseHandler::TypeTouch || mGesture->pointerType() == MouseHandler::TypeBoth){
            mTouchGen.doTouchUpdate(TasEventTarget(mGesture), points, mGesture->touchPointIdKey());
        }
        if(mGesture->pointerType() == MouseHandler::TypeMouse || mGesture->pointerType() == MouseHandler::TypeBoth){
            mMouseGen.doMouseMove(TasEventTarget(mGesture), points.first().screenPoint, mGesture->getMouseButton());
        }
    }
    else {
        for(int n=0;n<points.size();n++){
            TasTouchPoints touchpoint = points.at(n);
            mMouseGen.doMouseMove(TasEventTarget(mGesture),touchpoint.screenPoint,mGesture->getMouseButton(), n);
        }
    }

}

bool TasGestureRunner::eventFilter(QObject *target, QEvent *event)
{
    Q_UNUSED(target);
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

bool TasGestureRunner::noMovement(QList<TasTouchPoints> points)
{
    return mTouchGen.areIdentical(points, mPreviousPoints);
}
