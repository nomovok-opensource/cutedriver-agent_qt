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
 


#include <testabilityutils.h>

#include <QTimeLine>

#include "taslogger.h"
#include "gesturehandler.h"

/*!
  \class GestureHandler
  \brief GestureHandler generates mouse press and release events.

*/    

static int FRAME_RANGE_DIV = 10;


GestureHandler::GestureHandler(QObject* parent)
    :QObject(parent)
{
    mGesturePath = 0;
}

GestureHandler::~GestureHandler()
{    
    if(mGesturePath){
        delete mGesturePath;
    }
}

/*!
  Starts a gesture motion based on the given arguments from TasCommand. 
  The gesture is done using a mouse press, move and release operations.
  The path is determined from the given arguments and a QLineF or a list
  of points build from them.

  QTimeLine is used to make the gesture operations. The value from the valuechanged signal
  of timeline is used to determine the position.
 */
bool GestureHandler::executeInteraction(TargetData data)
{
    bool wasConsumed = false;

    mWidget = data.target;
    mItem = data.targetItem;
    TasCommand& command = *data.command;
    QPoint point = data.targetPoint;
    QWidget* target = data.target;
    QGraphicsItem* targetItem = data.targetItem;

    QString commandName = command.name();
    setPoint(point, command);
    if(mGesturePath){
        delete mGesturePath;
        mGesturePath = 0;
    }
    if (commandName == "MouseGesture"){  
        setParameters(command);
        QLineF gestureLine;
        gestureLine.setP1(QPointF(point));
        gestureLine.setLength(mDistance);
        gestureLine.setAngle(mDirection);
        if(targetItem){
            QGraphicsView* view = TestabilityUtils::getViewForItem(targetItem);
            if(view && !view->viewportTransform().isIdentity()){                    
                TasLogger::logger()->debug("Gesture is transformed");
                QTransform transform = view->viewportTransform();
                QLineF transformedLine = transform.map(gestureLine);
                //set new angle and length based on transformation
                gestureLine.setLength(transformedLine.length());
                gestureLine.setAngle(transformedLine.angle());
            }
        }
        mGesturePath = new LinePath(gestureLine);

        startGesture();
        wasConsumed = true;
    }
    else if(commandName == "MouseGestureTo"){     
        setParameters(command);
        QPoint end = getTargetPoint();
        if(!end.isNull()){
            QLineF gestureLine;
            gestureLine.setP1(QPointF(point));
            gestureLine.setP2(QPointF(end));
            mGesturePath = new LinePath(gestureLine);
            startGesture();
        }
        else{
            TasLogger::logger()->error("GestureHandler::executeInteraction could not find drag target!");
        }
        wasConsumed = true;
    }
    else if(commandName == "MouseGestureToCoordinates"){     
        setParameters(command);
        int x = command.parameter("x").toInt();
        int y = command.parameter("y").toInt();
        QPoint end(x,y);
        if(!end.isNull()){
            QLineF gestureLine;
            gestureLine.setP1(QPointF(point));
            gestureLine.setP2(QPointF(end));
            mGesturePath = new LinePath(gestureLine);
            startGesture();
        }
        else{
            TasLogger::logger()->error("GestureHandler::executeInteraction could not find drag target!");
        }
        wasConsumed = true;
    }
    else if(commandName == "MouseGesturePoints"){
        setParameters(command);
        QList<QPoint> gesturePoints;
        QList<int> gestureIntervals;
        //        gesturePoints.append(point);
        QStringList points = command.text().split(";",QString::SkipEmptyParts);
        for(int i = 0 ; i < points.size(); i++){
            QString rawPoint = points.at(i);
            QStringList pointData = rawPoint.split(",",QString::SkipEmptyParts);
            //skip invalid data
            //gestures done asynchronously so no possibility for error reporting
            if(pointData.size() >= 2){
                int x = pointData.at(0).toInt();
                int y = pointData.at(1).toInt();
                QPoint windowPoint(x,y);
                if(target->window()){
                    gesturePoints.append(target->window()->mapToGlobal(windowPoint));  
                }
                else{
                    gesturePoints.append(windowPoint);  
                }
                if(pointData.size() == 3){
                    gestureIntervals.append(pointData.at(2).toInt());
                }
            }
        }
        mGesturePath = new PointsPath(gesturePoints);
        mGesturePath->setIntervals(gestureIntervals);
        startGesture();
        wasConsumed = true;
    }
    return wasConsumed;
}

void GestureHandler::startGesture()
{
    QTimeLine* timeLine = new QTimeLine();
    timeLine->setCurveShape(QTimeLine::LinearCurve);
    connect(timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(timerEvent(qreal)));
    connect(timeLine, SIGNAL(finished()), this, SLOT(finished()));
    connect(timeLine, SIGNAL(finished()), timeLine, SLOT(deleteLater()));    

    //make the first press and move a bit to make sure that 
    //the gesture is not detected to be a long press
    qApp->installEventFilter(this);
    beginGesture();
    timeLine->setDuration(mDuration);
    timeLine->setFrameRange(0,mDuration/FRAME_RANGE_DIV);    
    timeLine->start();
}


/*!
    Move the pointer based on the direction and stepsize.
*/
void GestureHandler::timerEvent(qreal value)
{
    //do not send an event if no movement
    if(mPrevious != mGesturePath->pointAt(value)){
        doMouseMove(mWidget, mItem, mGesturePath->pointAt(value), mButton);        
    }
    mPrevious = mGesturePath->pointAt(value);
}

void GestureHandler::finished()
{
    //make sure the end is reached
    //do not send an event if no movement
    if(mPrevious != mGesturePath->endPoint()){
        doMouseMove(mWidget, mItem, mGesturePath->endPoint(), mButton);
    }
    if(mRelease){
        if(mIsDrag){
            //send move event to the end point with intention to cause a stopping effect
            doMouseMove(mWidget, mItem, mGesturePath->endPoint(), mButton);          
            //stay motionless and then release mouse
            QTimer::singleShot(50, this, SLOT(releaseMouse()));
        }
        else{
            //release immediately to cause a flick
            releaseMouse();
        }
    }
}

void GestureHandler::beginGesture()
{
    if(mPress){
        doMouseMove(mWidget, mItem, mGesturePath->startPoint(), Qt::NoButton);
        mPrevious = mGesturePath->startPoint();
        doMousePress(mWidget, mItem, mButton, mGesturePath->startPoint());            
    }

}

void GestureHandler::releaseMouse()
{
    doMouseRelease(mWidget, mItem, mButton, mGesturePath->endPoint());            
}

/*!
    MouseMove event will cause a mouse grab event with undesired effects.
    The filter filters the event to avoid it.
*/
bool GestureHandler::eventFilter(QObject* /*target*/, QEvent *event)
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

void GestureHandler::setParameters(TasCommand& command)
{
    if (!command.parameter("direction").isEmpty()) {
        mDirection = command.parameter("direction").toInt();
        mDirection -= 90;
        mDirection = mDirection * -1;
    }
    if (!command.parameter("distance").isEmpty()) {
        mDistance = command.parameter("distance").toInt();
    }    
    if (!command.parameter("speed").isEmpty()) {
        mDuration = command.parameter("speed").toInt();
    }
    if (!command.parameter("targetId").isEmpty()) {
        mTargetId = command.parameter("targetId");
    }
    if (!command.parameter("targetType").isEmpty()) {
        mTargetType = command.parameter("targetType");
    }

    mIsDrag = false;
    if (command.parameter("isDrag") == "true") {
        mIsDrag = true;
    }

    mButton = getMouseButton(command);
    
    mPress = true;
    mRelease = true;
    if (command.parameter("press") == "false") {
        mPress = false;
    }
    if (command.parameter("release") == "false") {
        mRelease = false;
    }
}

QPoint GestureHandler::getTargetPoint()
{
    QPoint targetPoint;
    if(mTargetType == TYPE_GRAPHICS_VIEW){
        QGraphicsItem* targetItem = findGraphicsItem(mTargetId); 
        if(targetItem){
            viewPortAndPosition(targetItem, targetPoint);   
        }
    }
    else{
        QWidget* targetWidget = findWidget(mTargetId);
        if(targetWidget){
            targetPoint = targetWidget->mapToGlobal(targetWidget->rect().center());
        }
    }
    return targetPoint;
}


/*!
  \class LinePath
  \brief Returns points on a coordinate line

  Mousegesture done in a line format are done unsing QLines.
  The line is divided in to sections of QPoints on the line. 
  The mouse is then moved along those points. 

*/    


LinePath::LinePath(QLineF gestureLine)
{
    mGestureLine = gestureLine;
}

QPoint LinePath::startPoint()
{
    return mGestureLine.p1().toPoint();
}

/*!
  Returns a point for the value. The value is something between 0 and 1.
 */
QPoint LinePath::pointAt(qreal value)
{
    return mGestureLine.pointAt(value).toPoint();
}

QPoint LinePath::endPoint()
{
    return mGestureLine.p2().toPoint();
}


PointsPath::PointsPath(QList<QPoint> points)
{
    mPoints = points;
    mUseIntervals = false;
}

/*
  Will return a null point of list empty.
 */
QPoint PointsPath::startPoint()
{
    if(mPoints.isEmpty()){
        return QPoint();
    }
    else{
        return mPoints.first();
    }
    
}

void PointsPath::setIntervals(QList<int> intervals)
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
QPoint PointsPath::pointAt(qreal value)
{

    if(mPoints.isEmpty()){
        return QPoint();
    }
    else{        
        int step = qRound(value*mPoints.size());
        if(step < mPoints.size()){
            return mPoints.at(step);
        }
        else{
            return endPoint();
        }        
    }
}

bool PointsPath::useIntervals()
{
    return mUseIntervals;
}

/*
  Will return a null point of list empty.
 */
QPoint PointsPath::endPoint()
{
    if(mPoints.isEmpty()){
        return QPoint();
    }
    else{
        return mPoints.last();
    }
}

int PointsPath::getDuration()
{
    int duration = 0;
    if(!mIntervals.isEmpty()){
        for (int i = 0; i < mIntervals.size(); ++i) {
            duration += mIntervals.at(i);
        }
    }
    return duration;
}


/*!
  QTimeLine send frameupdates at certain intervals. Therefore we need to multiply those
  points which have larget interval values.
 */
void PointsPath::calculateAnimation()
{
    int duration = getDuration();
    if(duration > 0){
        int frames = qRound(duration/FRAME_RANGE_DIV);
        int avgFrameTime = qRound(duration/frames);
        QList<QPoint> timedPoints;
        //go through each point and check the interval
        for(int i = 0 ; i < mPoints.size(); i++){
            QPoint point = mPoints.at(i);
            int interval = mIntervals.at(i);
            int multiplier = qRound(interval/avgFrameTime);
            timedPoints.append(point); // all points are added once (at least)
            multiplier--;
            for(int j = 0 ; j < multiplier; j++){
                timedPoints.append(point);   
            }
        }
        mPoints = timedPoints;
    }
}
