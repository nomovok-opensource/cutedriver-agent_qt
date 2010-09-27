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

TasGestureRunner::TasGestureRunner(TasGesture* gesture, QObject* parent)
    :QObject(parent)
{
    mGesture = gesture;
    connect(&mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(timerEvent(qreal)));
    connect(&mTimeLine, SIGNAL(finished()), this, SLOT(finished()));
    qApp->installEventFilter(this);
    startGesture();
}

TasGestureRunner::~TasGestureRunner()
{
    delete mGesture;
}

void TasGestureRunner::startGesture()
{
    int duration = mGesture->getDuration();
    mTimeLine.setDuration(duration);
    mTimeLine.setFrameRange(0,duration/FRAME_RANGE_DIV);    
    mTimeLine.start();
    if(mGesture->isPress()){
        if(mGesture->isMultiTouch()){
            mTouchGen.doTouchBegin(mGesture->getTarget(), mGesture->getTargetItem(), mGesture->startPoints());       
        }
        else{
            mPreviousPoints = mGesture->startPoints();
            if(mGesture->pointerType() == MouseHandler::TypeMouse || mGesture->pointerType() == MouseHandler::TypeBoth){
                mMouseGen.doMouseMove(mGesture->getTarget(), mGesture->startPoints().first().screenPoint, mGesture->getMouseButton());
                mMouseGen.doMousePress(mGesture->getTarget(), mGesture->getMouseButton(), mGesture->startPoints().first().screenPoint);
            }
            if(mGesture->pointerType() == MouseHandler::TypeTouch || mGesture->pointerType() == MouseHandler::TypeBoth){
                mTouchGen.doTouchBegin(mGesture->getTarget(), mGesture->getTargetItem(), mGesture->startPoints());       
            }            
        }
    }
}

void TasGestureRunner::timerEvent(qreal value)
{
    move(mGesture->pointsAt(value));
    mPreviousPoints = mGesture->pointsAt(value);
}
void TasGestureRunner::finished()
{
    move(mGesture->endPoints());
    if(mGesture->isRelease()){
        //send move event to the end point with intention to cause a stopping effect
        if(mGesture->isDrag()){            
            move(mGesture->endPoints(), true);
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
    if(mGesture->isMultiTouch() && mGesture->isRelease()){
        mTouchGen.doTouchEnd(mGesture->getTarget(), mGesture->getTargetItem(), mGesture->endPoints());       
    }
    else{
        if(mGesture->pointerType() == MouseHandler::TypeMouse || mGesture->pointerType() == MouseHandler::TypeBoth){
            mMouseGen.doMouseRelease(mGesture->getTarget(), mGesture->getMouseButton(), mGesture->endPoints().first().screenPoint);
        }        
        if(mGesture->pointerType() == MouseHandler::TypeTouch || mGesture->pointerType() == MouseHandler::TypeBoth){
            mTouchGen.doTouchEnd(mGesture->getTarget(), mGesture->getTargetItem(), mGesture->endPoints());       
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
    if(mGesture->isMultiTouch()){
        mTouchGen.doTouchUpdate(mGesture->getTarget(), mGesture->getTargetItem(), points);       
    }
    else{
        if(mGesture->pointerType() == MouseHandler::TypeMouse || mGesture->pointerType() == MouseHandler::TypeBoth){
            mMouseGen.doMouseMove(mGesture->getTarget(), points.first().screenPoint, mGesture->getMouseButton());
        }        
        if(mGesture->pointerType() == MouseHandler::TypeTouch || mGesture->pointerType() == MouseHandler::TypeBoth){
            mTouchGen.doTouchUpdate(mGesture->getTarget(), mGesture->getTargetItem(), points);       
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
    if(mPreviousPoints.size() != points.size()){
        return false;
    }
    //loop points to detect differences
    for(int i = 0 ; i < points.size() ; i++){
        TasTouchPoints t = points.at(i);
        TasTouchPoints p = mPreviousPoints.at(i);
        if(p.screenPoint != t.screenPoint || t.lastScreenPoint != p.lastScreenPoint ||
           p.startScreenPoint != t.startScreenPoint){
            return false;
        }
    }
    return true;
}
