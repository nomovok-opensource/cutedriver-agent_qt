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
#include "taslogger.h"
#include "tasmultigesturerunner.h" 


TasMultiGestureRunner::TasMultiGestureRunner(QList<TasGesture*> gestures, QObject* parent)
  :QObject(parent), mUseTapScreen(false)
{    
    connect(&mTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(gestureTimerEvent(qreal)));
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

    foreach(TasGesture* gesture, gestures){
        if(gesture->getUseTapScreen()){
            mMouseGen.setUseTapScreen(true);
            mUseTapScreen = true;
        }
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

    //only 1 target widget supported for now
    QWidget* target = mGestures.keys().first()->getTarget();

    //use qt touch event or send mouse event (native)
    if(!mUseTapScreen) {
        // TODO [hhinrich] delete touchDevice?
        QTouchDevice* touchDevice = new QTouchDevice();
        touchDevice->setType(QTouchDevice::TouchScreen);
        QTouchEvent* touchPress = new QTouchEvent(QEvent::TouchBegin, touchDevice, Qt::NoModifier,
                                                  Qt::TouchPointPressed, touchPoints);
        touchPress->setTarget(target);
        mTouchGen.sendTouchEvent(target, touchPress);
    } else {
        //using 'n' as a pointer number
        for(int n=0;n<touchPoints.size();n++){
          QPoint point((int)touchPoints.at(n).pos().x(),(int)touchPoints.at(n).pos().y());
          mMouseGen.doMousePress(target,Qt::LeftButton,point, n);
        }
    }

    mTimeLine.start();
}

void TasMultiGestureRunner::gestureTimerEvent(qreal value)
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

    //only 1 target widget supported for now
    QWidget* target = mGestures.keys().first()->getTarget();

    //use qt touch event or send mouse event (native)
    if(!mUseTapScreen) {
        // TODO [hhinrich] delete touchDevice?
        QTouchDevice* touchDevice = new QTouchDevice();
        touchDevice->setType(QTouchDevice::TouchScreen);
        QTouchEvent* touchEvent = new QTouchEvent(QEvent::TouchUpdate, touchDevice, Qt::NoModifier,
                                                  states, touchPoints);
        touchEvent->setTarget(target);
        mTouchGen.sendTouchEvent(target, touchEvent);
    } else {
        //using 'n' as a pointer number
        for(int n=0;n<touchPoints.size();n++){
          QPoint point((int)touchPoints.at(n).pos().x(),(int)touchPoints.at(n).pos().y());
          mMouseGen.doMouseMove(target,point,Qt::LeftButton, n);
        }
    }

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

    //use qt touch event or send mouse event (native)
    if(!mUseTapScreen) {
        // TODO [hhinrich] delete touchDevice?
        QTouchDevice* touchDevice = new QTouchDevice();
        touchDevice->setType(QTouchDevice::TouchScreen);
        QTouchEvent* touchEvent = new QTouchEvent(QEvent::TouchUpdate, touchDevice, Qt::NoModifier,
                                                  Qt::TouchPointMoved, touchPoints);
        touchEvent->setTarget(target);
        mTouchGen.sendTouchEvent(target, touchEvent);

        //2. Send releases when needed
        QTouchEvent* releaseEvent = new QTouchEvent(QEvent::TouchEnd, touchDevice, Qt::NoModifier,
                                                  Qt::TouchPointReleased, touchReleasePoints);
        releaseEvent->setTarget(target);
        mTouchGen.sendTouchEvent(target, releaseEvent);
    } else {
        //using 'n' as a pointer number
        for(int n=0;n<touchPoints.size();n++){
              QPoint point((int)touchPoints.at(n).pos().x(),(int)touchPoints.at(n).pos().y());
              mMouseGen.doMouseMove(target,point,Qt::LeftButton, n);
        }
        for(int n=0;n<touchReleasePoints.size();n++){
            QPoint point((int)touchReleasePoints.at(n).pos().x(),(int)touchReleasePoints.at(n).pos().y());
            mMouseGen.doMouseRelease(target,Qt::LeftButton,point, n);
        }
    }
    deleteLater();
}

