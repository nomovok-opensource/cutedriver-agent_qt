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

#include "tasgesture.h"
#include "taslogger.h"
#include "mousehandler.h"

/*!
    \class TasGesture

    \brief TasGesture provides the details for the gesture simulating events.
    
    TasGesture provides the TasGestureRunner the require details to simulate a 
    gesture on an object. The details include the points, duration and speed of 
    the gesture. TasGestures may be multitouch type which means that touchpoints 
    with multiple touch points will be generated for each touch event.
*/
/*!
    \fn void TasGesture::startPoints()

    Returns the starting point(s) of the gesture. Multiple points are used in 
    multitouch situtations.
*/

/*!
    \fn void TasGesture::pointAts(qreal value)

    Returns the point(s) from the given point. The value is something between
    0 and 1. 0 is the starting point and 1 the end.
*/

/*!
    \fn void TasGesture::endPoint()

    Return the end point(s) of the gesture. 
*/

/*!
    \fn void TasGesture::isMultiTouch()

    Is the gesture a multitouch gesture or not.

 */

/*!
  Base constructor for TasGesture. Sets the basic 
  command details for all gestures.
 */
TasGesture::TasGesture(TargetData data)
{
    mPointerType = MouseHandler::TypeMouse;
    

    mTarget = data.target;
    mTargetItem = data.targetItem;
    if(mTargetItem){
        mTouchPointIdKey = TasCoreUtils::pointerId(mTargetItem);
    }
    else{
        mTouchPointIdKey = TasCoreUtils::objectId(mTarget);
    }
    TasCommand& command = *data.command;

    if (!command.parameter("speed").isEmpty()) {
        mDuration = command.parameter("speed").toInt();
    }

    mIsDrag = false;
    if (command.parameter("isDrag") == "true") {
        mIsDrag = true;
    }

    mPress = true;
    mRelease = true;
    if (command.parameter("press") == "false") {
        mPress = false;
    }
    if (command.parameter("release") == "false") {
        mRelease = false;
    }
    if(!command.parameter(POINTER_TYPE).isEmpty()){
        setPointerType(static_cast<MouseHandler::PointerType>(command.parameter(POINTER_TYPE).toInt()));
    }
    //if we know that the gesture a complete one (with press and release)
    //set the id to be unique for the gesture
    if(mPress && mRelease){
        mTouchPointIdKey.append(QString::number(qrand()));
    }

    mButton = MouseHandler::getMouseButton(command);
}

/*!
  Returns the pointer type to be used for the gesture. The type will not be changed 
  during the gesture. The type determines that will mouse or touch events be generated
  to generate the gesture.
 */
MouseHandler::PointerType TasGesture::pointerType()
{
    return mPointerType;
}

Qt::MouseButton TasGesture::getMouseButton()
{
    return mButton;
}

/*!
  Set the pointer type to be used for the gesture. The type will not be changed 
  during the gesture. The type determines that will mouse or touch events be generated
  to generate the gesture.
 */
void TasGesture::setPointerType(MouseHandler::PointerType type)
{
    mPointerType = type;
}

/*!
  Return the target for the gesture. Usually the viewport. 
 */
QWidget* TasGesture::getTarget()
{
    return mTarget;
}

/*!
  Returns the item to which the gesture is targetted to. Can be null if no item.
 */
QGraphicsItem* TasGesture::getTargetItem()
{
    return mTargetItem;
}

/*!
  Some touchevents handling requires that the start point and last position are delivered with 
  the event. Generates a struct of the points.
 */ 
TasTouchPoints TasGesture::makeTouchPoint(QPoint pos, QPoint lastPos, QPoint startPos)
{
    TasTouchPoints touchPoint;
    touchPoint.screenPoint = pos;
    touchPoint.lastScreenPoint = lastPos;
    touchPoint.startScreenPoint = startPos;
    return touchPoint;
}

/*!
  Util function for making a list from one set of points.
 */
QList<TasTouchPoints> TasGesture::listFromPoint(QPoint pos, QPoint lastPos, QPoint startPos)
{
    QList<TasTouchPoints> list;
    list.append(makeTouchPoint(pos, lastPos, startPos));
    return list;
}

/*!
  \class LineTasGesture
  \brief Returns points on a coordinate line

  Mousegesture done in a line format are done unsing QLines.
  The line is divided in to sections of QPoints on the line. 
  The mouse is then moved along those points. 

*/    

LineTasGesture::LineTasGesture(TargetData data, QLineF gestureLine)
    :TasGesture(data)
{
    mGestureLine = gestureLine;
}

QList<TasTouchPoints> LineTasGesture::startPoints()
{
    mStartPoint = mGestureLine.p1().toPoint();
    mLastPoint = mStartPoint;
    return listFromPoint(mStartPoint);
}

/*!
  Returns a point for the value. The value is something between 0 and 1.
 */
QList<TasTouchPoints> LineTasGesture::pointsAt(qreal value)
{
    QPoint current = mGestureLine.pointAt(value).toPoint();
    QList<TasTouchPoints> list = listFromPoint(current, mLastPoint, mStartPoint);
    mLastPoint = current;
    return list;
}

QList<TasTouchPoints> LineTasGesture::endPoints()
{
    return listFromPoint(mGestureLine.p2().toPoint(), mLastPoint, mStartPoint);
}

PointsTasGesture::PointsTasGesture(TargetData data, QList<QPoint> points)
    :TasGesture(data)
{
    mPoints = points;
    mUseIntervals = false;
}

/*
  Will return a null point of list empty.
 */
QList<TasTouchPoints> PointsTasGesture::startPoints()
{    
    if(!mPoints.isEmpty()){
        mStartPoint = mPoints.first();
    }
    mLastPoint = mStartPoint;
    return listFromPoint(mStartPoint);
}

void PointsTasGesture::setIntervals(QList<int> intervals)
{
    mIntervals = intervals;
    mUseIntervals = true;
    calculateAnimation();
}


/*!
  Returns a point for the value. The value is something between 0 and 1. 
  Calculated the correct point for the value. Will return a null point of 
  list empty.
 */
QList<TasTouchPoints> PointsTasGesture::pointsAt(qreal value)
{
    QPoint current;
    if(!mPoints.isEmpty()){
        int step = qRound(value*mPoints.size());
        if(step < mPoints.size()){
            current = mPoints.at(step);
        }
        else{
            current = mPoints.last();
        }        
    }
    QList<TasTouchPoints> list = listFromPoint(current, mLastPoint, mStartPoint);
    mLastPoint = current;
    return list;
}

/*
  Will return a null point of list empty.
 */
QList<TasTouchPoints> PointsTasGesture::endPoints()
{
    QPoint current;
    if(!mPoints.isEmpty()){
        current = mPoints.last();
    }
    return listFromPoint(current, mLastPoint, mStartPoint);
}

int PointsTasGesture::getDuration()
{
    int duration = 0;
    if(!mIntervals.isEmpty()){
        for (int i = 0; i < mIntervals.size(); ++i) {
            duration += mIntervals.at(i);
        }
    }
    if (duration < FRAME_RANGE_DIV) {
        return FRAME_RANGE_DIV;
    }
    return duration;
}


/*!
  QTimeLine send frameupdates at certain intervals. Therefore we need to multiply those
  points which have larget interval values.
 */
void PointsTasGesture::calculateAnimation()
{
    int duration = getDuration();
    if(duration > 0){
        int frames = qRound((qreal)(duration/FRAME_RANGE_DIV));
        int avgFrameTime = qRound((qreal)(duration/frames));
        QList<QPoint> timedPoints;
        //go through each point and check the interval
        for(int i = 0 ; i < mPoints.size(); i++){
            QPoint point = mPoints.at(i);
            int interval = mIntervals.at(i);
            int multiplier = qRound((qreal)(interval/avgFrameTime));
            timedPoints.append(point); // all points are added once (at least)
            multiplier--;
            for(int j = 0 ; j < multiplier; j++){
                timedPoints.append(point);   
            }
        }
        mPoints = timedPoints;
    }
}

PinchZoomTasGesture::PinchZoomTasGesture(TargetData data, QLineF line1, QLineF line2)
    :TasGesture(data)
{
    mGestureLine1 = line1;
    mGestureLine2 = line2;
    mPointerType = MouseHandler::TypeTouch;
}

PinchZoomTasGesture::~PinchZoomTasGesture()
{}

QList<TasTouchPoints> PinchZoomTasGesture::startPoints()
{
    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mGestureLine1.p1().toPoint()));
    points.append(makeTouchPoint(mGestureLine2.p1().toPoint()));
    mLastPoints.clear();
    mLastPoints.append(mGestureLine1.p1().toPoint());
    mLastPoints.append(mGestureLine2.p1().toPoint());
    mStartPoints = QList<QPoint>(mLastPoints);
    return points;
}

QList<TasTouchPoints> PinchZoomTasGesture::pointsAt(qreal value)
{
    QList<TasTouchPoints> points;
    TasTouchPoints touchPoint1 = makeTouchPoint(mGestureLine1.pointAt(value).toPoint(), mLastPoints.at(0), mStartPoints.at(0));
    TasTouchPoints touchPoint2 = makeTouchPoint(mGestureLine2.pointAt(value).toPoint(), mLastPoints.at(1), mStartPoints.at(1));
    points.append(touchPoint1);
    points.append(touchPoint2);
    mLastPoints.clear();
    mLastPoints.append(touchPoint1.screenPoint);
    mLastPoints.append(touchPoint2.screenPoint);
    return points;
}

QList<TasTouchPoints> PinchZoomTasGesture::endPoints()
{
    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mGestureLine1.p2().toPoint(), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(mGestureLine2.p2().toPoint(), mLastPoints.at(1), mStartPoints.at(1)));
    return points;
}

SectorTasGesture::SectorTasGesture(TargetData data, QLineF line, int distance)
    :TasGesture(data)
{
    mPointerType = MouseHandler::TypeTouch;
    mGestureLine = line;
    mStartAngle = mGestureLine.angle();
    mDistance = distance;
}

QList<TasTouchPoints> SectorTasGesture::startPoints()
{
    mLastPoints = activePoints();
    mStartPoints = QList<QPoint>(mLastPoints);

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mLastPoints.at(0)));
    points.append(makeTouchPoint(mLastPoints.at(1)));
    return points;
}

QList<QPoint> SectorTasGesture::activePoints()
{
    QList<QPoint> points;
    points.append(mGestureLine.p1().toPoint());
    points.append(mGestureLine.p2().toPoint());    
    return points;
}

QList<TasTouchPoints> SectorTasGesture::pointsAt(qreal value)
{
    mGestureLine.setAngle((mDistance*value)+mStartAngle);
    QList<QPoint> currentPoints = activePoints();

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(currentPoints.at(0), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(currentPoints.at(1), mLastPoints.at(1), mStartPoints.at(1)));
    mLastPoints = currentPoints;

    return points;
}

QList<TasTouchPoints> SectorTasGesture::endPoints()
{
    mGestureLine.setAngle(mDistance+mStartAngle);
    QList<QPoint> currentPoints = activePoints();

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(currentPoints.at(0), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(currentPoints.at(1), mLastPoints.at(1), mStartPoints.at(1)));
    mLastPoints = currentPoints;

    return points;
}

ArcsTasGesture::ArcsTasGesture(TargetData data, QLineF line1, QLineF line2, int distance)
    :TasGesture(data)
{
    mGestureLine1 = line1;
    mGestureLine2 = line2;
    mStartAngle1 = mGestureLine1.angle();
    mStartAngle2 = mGestureLine2.angle();
    mDistance = distance;
    mPointerType = MouseHandler::TypeTouch;
}

QList<TasTouchPoints> ArcsTasGesture::startPoints()
{    
    mLastPoints = activePoints();
    mStartPoints = QList<QPoint>(mLastPoints);

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mLastPoints.at(0)));
    points.append(makeTouchPoint(mLastPoints.at(1)));
    return points;
}

QList<QPoint> ArcsTasGesture::activePoints()
{
    QList<QPoint> points;
    points.append(mGestureLine1.p2().toPoint());
    points.append(mGestureLine2.p2().toPoint());
    return points;
}

QList<TasTouchPoints> ArcsTasGesture::pointsAt(qreal value)
{
    mGestureLine1.setAngle((mDistance*value)+mStartAngle1);
    mGestureLine2.setAngle((mDistance*value)+mStartAngle2);
    QList<QPoint> currentPoints = activePoints();
    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(currentPoints.at(0), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(currentPoints.at(1), mLastPoints.at(1), mStartPoints.at(1)));
    mLastPoints = currentPoints;
    return points;
}

QList<TasTouchPoints> ArcsTasGesture::endPoints()
{
    mGestureLine1.setAngle(mDistance+mStartAngle1);
    mGestureLine2.setAngle(mDistance+mStartAngle2);
    QList<QPoint> currentPoints = activePoints();

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(currentPoints.at(0), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(currentPoints.at(1), mLastPoints.at(1), mStartPoints.at(1)));
    mLastPoints = currentPoints;
    return points;
}

