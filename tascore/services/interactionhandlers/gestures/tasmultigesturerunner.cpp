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

#include "tasmultigesturerunner.h" 


TasMultiGestureRunner::TasMultiGestureRunner(QList<TasGesture*> gestures, QObject* parent)
    :QObject(parent)
{    
    connect(&mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(timerEvent(qreal)));
    connect(&mTimeLine, SIGNAL(finished()), this, SLOT(finished()));
    //calculate duration multipliers
    int maxDuration = 0;
    foreach(TasGesture* gesture, gestures){
        if(gesture->getDuration() > maxDuration){
            maxDuration = gesture->getDuration();
        }
    }
    foreach(TasGesture* gesture, gestures){
        mGestures.insert(gesture, maxDuration/gesture->getDuration());
    }

    mTimeLine.setDuration(maxDuration);
    mTimeLine.setFrameRange(0, maxDuration/FRAME_RANGE_DIV);    

    startGesture();
}

TasMultiGestureRunner::~TasMultiGestureRunner()
{
    qDeleteAll(mGestures.keys());
    mGestures.clear();
}

void TasMultiGestureRunner::startGesture()
{
    QList<QTouchEvent::TouchPoint> touchPoints;
    //collect press points
    foreach(TasGesture* gesture, mGestures.keys()){
        if(gesture->isPress()){
            touchPoints.append(mTouchGen.convertToTouchPoints(gesture->getTarget(), Qt::TouchPointPressed, 
                                                              gesture->startPoints(), gesture->touchPointIdKey()));
        }
        mPreviousPoints.insert(gesture, gesture->startPoints());
    }
    QTouchEvent* touchPress = new QTouchEvent(QEvent::TouchBegin, QTouchEvent::TouchScreen, Qt::NoModifier, 
                                              Qt::TouchPointPressed, touchPoints);
    //only 1 target widget supported for now
    QWidget* target = mGestures.keys().first()->getTarget();
    touchPress->setWidget(target);
    mTouchGen.sendTouchEvent(target, touchPress);    

    mTimeLine.start();
}

void TasMultiGestureRunner::timerEvent(qreal value)
{
    QList<TasGesture*> finished;
    Qt::TouchPointStates states = Qt::TouchPointMoved;
    QList<QTouchEvent::TouchPoint> touchPoints;
    foreach(TasGesture* gesture, mGestures.keys()){
        qreal correctedValue = value*mGestures.value(gesture);
        //gesture ending send release and mark gesture to be removed
        if(correctedValue > 1){
            finished.append(gesture);
            if(gesture->isRelease()){
                states |= Qt::TouchPointReleased;
                touchPoints.append(mTouchGen.convertToTouchPoints(gesture->getTarget(), Qt::TouchPointReleased, 
                                                                  gesture->endPoints(), gesture->touchPointIdKey()));            
            }            
        }
        else if(!mTouchGen.areIdentical(gesture->pointsAt(correctedValue), mPreviousPoints.value(gesture))){
            touchPoints.append(mTouchGen.convertToTouchPoints(gesture->getTarget(), Qt::TouchPointMoved, 
                                                              gesture->pointsAt(correctedValue), gesture->touchPointIdKey()));
        }
        mPreviousPoints.insert(gesture, gesture->pointsAt(correctedValue));
    }
    QTouchEvent* touchEvent = new QTouchEvent(QEvent::TouchUpdate, QTouchEvent::TouchScreen, Qt::NoModifier, 
                                              states, touchPoints);    
    //only 1 target widget supported for now
    QWidget* target = mGestures.keys().first()->getTarget();
    touchEvent->setWidget(target);
    mTouchGen.sendTouchEvent(target, touchEvent);    

    //remove ended gestures 
    foreach(TasGesture* gesture, finished){
        mGestures.remove(gesture);
        mPreviousPoints.remove(gesture);
    }    
    qDeleteAll(finished);
}

void TasMultiGestureRunner::finished()
{
    //1. make sure all gestures reach the end point
    QList<QTouchEvent::TouchPoint> touchPoints;
    QList<QTouchEvent::TouchPoint> touchReleasePoints;
    foreach(TasGesture* gesture, mGestures.keys()){
        if(!mTouchGen.areIdentical(gesture->endPoints(), mPreviousPoints.value(gesture))){
            touchPoints.append(mTouchGen.convertToTouchPoints(gesture->getTarget(), Qt::TouchPointMoved, 
                                                              gesture->endPoints(), gesture->touchPointIdKey()));
        }
        if(gesture->isRelease()){
            touchReleasePoints.append(mTouchGen.convertToTouchPoints(gesture->getTarget(), Qt::TouchPointReleased, 
                                                                     gesture->endPoints(), gesture->touchPointIdKey()));
        }
    }

    //only 1 target widget supported for now
    QWidget* target = mGestures.keys().first()->getTarget();

    QTouchEvent* touchEvent = new QTouchEvent(QEvent::TouchUpdate, QTouchEvent::TouchScreen, Qt::NoModifier, 
                                              Qt::TouchPointMoved, touchPoints);    
    touchEvent->setWidget(target);
    mTouchGen.sendTouchEvent(target, touchEvent);    

    //2. Send releases when needed
    QTouchEvent* releaseEvent = new QTouchEvent(QEvent::TouchEnd, QTouchEvent::TouchScreen, Qt::NoModifier, 
                                              Qt::TouchPointReleased, touchReleasePoints);    
    releaseEvent->setWidget(target);
    mTouchGen.sendTouchEvent(target, releaseEvent);    
    deleteLater();
}


