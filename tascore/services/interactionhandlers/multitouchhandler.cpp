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

#include "taslogger.h"
#include "multitouchhandler.h"

static QString DIST_ONE = "distance_1"; 
static QString DIST_TWO = "distance_2"; 
static QString DIFF     = "differential";
static QString TYPE     = "type";
static QString RADIUS   = "radius";
static QString ANGLE    = "angle";
static QString ROTATE_DIRECTION = "rotate_direction";

/*!
  \class MultitouchHandler
  \brief MultitouchHandler generates mouse press and release events.

*/    


MultitouchHandler::MultitouchHandler(QObject* parent)
    :GestureHandler(parent)
{
    mGesturePath = 0;
}

MultitouchHandler::~MultitouchHandler()
{    
    if(mGesturePath){
        delete mGesturePath;
    }
}

/*!
  Starts a multitouch gesture motion based on the given arguments from TasCommand. 
  The gesture is done using a mouse press, move and release operations.
  The path is determined from the given arguments and a QLineF or a list
  of points build from them.

  QTimeLine is used to make the gesture operations. The value from the valuechanged signal
  of timeline is used to determine the position.
 */
bool MultitouchHandler::executeInteraction(TargetData data)
{
    bool wasConsumed = false;
    TasCommand& command = *data.command;
    QPoint point = data.targetPoint;
    QString commandName = command.name();
    mWidget = data.target;
    mItem = data.targetItem;

    if(commandName == "PinchZoom" || commandName == "Rotate"){
        //from mousehandler
        setPoint(point, command);
        //from gesturehandler
        setParameters(command);
        bool start = false;
        if(commandName == "PinchZoom"){                             
            start = makePinchZoomGesture(command, point);
        }
        else if(commandName == "Rotate"){
            start = makeRotationGesture(command, point);
        }
        if(start){
            startGesture();
        }
       wasConsumed = true;
    }
    return wasConsumed;
}

bool MultitouchHandler::executeMultitouchInteraction(QList<TargetData> dataList)
{
    bool consumed = false;
    if(!dataList.isEmpty()){
        consumed = true;
        //look for tap downs and taps (press down motion to start the multitouch)
        QList<QTouchEvent::TouchPoint> touchPoints;
        QList<QTouchEvent::TouchPoint> touchReleasePoints;
        TargetData targetData;
        foreach(targetData, dataList){
            if(targetData.command->name() == "MouseClick" || targetData.command->name() == "MousePress" ||
               targetData.command->name() == "Tap" ){            
                touchPoints.append(convertToTouchPoints(targetData.target,targetData.targetItem,
                                                        Qt::TouchPointPressed, toTouchPoints(targetData.targetPoint)));
            }
            if(targetData.command->name() == "MouseClick" || targetData.command->name() == "MouseRelease" ||
               targetData.command->name() == "Tap" ){            
                touchReleasePoints.append(convertToTouchPoints(targetData.target,targetData.targetItem,
                                                               Qt::TouchPointReleased, toTouchPoints(targetData.targetPoint)));
            }            
        }

        //currently only one target widget supported
        QWidget *target = dataList.first().target;

        //send begin event
        if(!touchPoints.isEmpty()){
            QTouchEvent* touchPress = new QTouchEvent(QEvent::TouchBegin, QTouchEvent::TouchScreen, Qt::NoModifier, 
                                                      Qt::TouchPointPressed, touchPoints);
            touchPress->setWidget(target);
            sendTouchEvent(target, touchPress);
 
        }
        else{
            TasLogger::logger()->debug("MultitouchHandler::executeMultitouchInteraction not tap downs");
        }

        if(!touchReleasePoints.isEmpty()){
            //send end event
            QTouchEvent *touchRelease = new QTouchEvent(QEvent::TouchEnd, QTouchEvent::TouchScreen, Qt::NoModifier, 
                                                        Qt::TouchPointReleased, touchReleasePoints);
            touchRelease->setWidget(target);
            sendTouchEvent(target, touchRelease);            
        }
        else{
            TasLogger::logger()->debug("MultitouchHandler::executeMultitouchInteraction not tap ups");            
        }
    }
    return consumed;
}

bool MultitouchHandler::makePinchZoomGesture(TasCommand& command, QPoint point)
{
    if(!validateZoomParams(command)){
        return false;
    }           
    int distance_1 = command.parameter(DIST_ONE).toInt();
    int distance_2 = command.parameter(DIST_TWO).toInt();
    int differential = command.parameter(DIFF).toInt();        

    //this is the diff line
    QLineF line = makeLine(point,differential/2, mDirection);

    //actual gesture lines
    QLineF line1 = makeLine(line.p2().toPoint(), distance_1, line.angle());
    line.setAngle(mDirection+180);
    QLineF line2 = makeLine(line.p2().toPoint(), distance_2, line.angle());

    if(mGesturePath){
        delete mGesturePath;
        mGesturePath = 0;
    }
    if(command.parameter(TYPE) == "in"){
        mGesturePath = new PinchZoomGesturePath(line1, line2);
    }
    else if(command.parameter(TYPE) == "out"){
        //flip the lines if zoom out
        mGesturePath = new PinchZoomGesturePath(QLineF(line1.p2(), line1.p1()), QLineF(line2.p2(), line2.p1()));
    }
    else{
        TasLogger::logger()->error("MultitouchHandler::makePinchZoomGesture invalid type.");
        return false;
    }
    return true;
}

bool MultitouchHandler::makeRotationGesture(TasCommand& command, QPoint point)
{
    if(!validateRotationParams(command)){
        return false;
    }
    int radius = command.parameter(RADIUS).toInt();
    if(command.parameter(ROTATE_DIRECTION) == "Clockwise"){
        mDistance = mDistance * -1;
    }

    QLineF line = makeLine(point, radius, mDirection);
    if(command.parameter(TYPE) == "one_point"){
        mGesturePath = new SectorGesturePath(line, mDistance);
    }
    else if(command.parameter(TYPE) == "two_point"){
        QLineF line2 = makeLine(point, radius, mDirection+180);
        mGesturePath = new ArcsGesturePath(line, line2, mDistance);
    }
    else{
        TasLogger::logger()->error("MultitouchHandler::makeRotationGesture invalid type.");
        return false;
    }
    return true;
}

QLineF MultitouchHandler::makeLine(QPoint start, int length, int angle)
{
    QLineF line;
    line.setP1(start);
    line.setLength(length);
    line.setAngle(angle);
    return line;
}

bool MultitouchHandler::validateZoomParams(TasCommand& command)
{
    bool valid = true;
    if(command.parameter(DIST_ONE).isEmpty() || command.parameter(DIST_TWO).isEmpty()) {
        TasLogger::logger()->error("MultitouchHandler::validateZoomParams invalid pinch command given invalid directions.");
        valid = false;        
    }    
    if(command.parameter(TYPE).isEmpty()){
        TasLogger::logger()->error("MultitouchHandler::validateZoomParams no type defined.");
        valid = false;        
    }
    if(command.parameter(DIFF).isEmpty()) {
        TasLogger::logger()->error("MultitouchHandler::validateZoomParams no differential defined.");
        valid = false;        
    }
    return valid;
}

bool MultitouchHandler::validateRotationParams(TasCommand& command)
{
    bool valid = true;
    if(command.parameter(TYPE).isEmpty()){
        TasLogger::logger()->error("MultitouchHandler::validateRotationParams no type defined.");
        valid = false;        
    }
    if(command.parameter(RADIUS).isEmpty()) {
        TasLogger::logger()->error("MultitouchHandler::executeInteraction no radius defined.");
        valid = false;        
    }
    if(command.parameter(ROTATE_DIRECTION).isEmpty()) {
        TasLogger::logger()->error("MultitouchHandler::executeInteraction no rotation direction defined.");
        valid = false;        
    }
    return valid;
}


/*!
    Update gesture touch points
*/
void MultitouchHandler::timerEvent(qreal value)
{
    doTouchUpdate(mWidget, mItem, mGesturePath->pointAts(value));
}

void MultitouchHandler::finished()
{
    //make sure the end is reached
    doTouchUpdate(mWidget, mItem, mGesturePath->endPoints());
    doTouchEnd(mWidget, mItem, mGesturePath->endPoints());
}

void MultitouchHandler::beginGesture()
{
    doTouchBegin(mWidget, mItem, mGesturePath->startPoints());
}

TasTouchPoints MultitouchGesturePath::makeTouchPoint(QPoint pos, QPoint lastPos, QPoint startPos)
{
    TasTouchPoints touchPoint;
    touchPoint.screenPoint = pos;
    touchPoint.lastScreenPoint = lastPos;
    touchPoint.startScreenPoint = startPos;
    return touchPoint;
}

PinchZoomGesturePath::PinchZoomGesturePath(QLineF line1, QLineF line2)
{
    mGestureLine1 = line1;
    mGestureLine2 = line2;
}

PinchZoomGesturePath::~PinchZoomGesturePath()
{}

QList<TasTouchPoints> PinchZoomGesturePath::startPoints()
{
    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mGestureLine1.p1().toPoint()));
    points.append(makeTouchPoint(mGestureLine2.p1().toPoint()));
    mLastPoints.clear();
    mLastPoints.append(mGestureLine1.p1().toPoint());
    mLastPoints.append(mGestureLine1.p2().toPoint());
    mStartPoints = QList<QPoint>(mLastPoints);
    return points;
}

QList<TasTouchPoints> PinchZoomGesturePath::pointAts(qreal value)
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

QList<TasTouchPoints> PinchZoomGesturePath::endPoints()
{
    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mGestureLine1.p2().toPoint(), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(mGestureLine2.p2().toPoint(), mLastPoints.at(1), mStartPoints.at(1)));
    return points;
}

SectorGesturePath::SectorGesturePath(QLineF line, int distance)
{
    mGestureLine = line;
    mStartAngle = mGestureLine.angle();
    mDistance = distance;
    TasLogger::logger()->debug("SectorGesturePath::SectorGesturePath line: " + QString::number(mGestureLine.angle()) + 
                               "," + QString::number(mGestureLine.length()));
}

QList<TasTouchPoints> SectorGesturePath::startPoints()
{
    mLastPoints = activePoints();
    mStartPoints = QList<QPoint>(mLastPoints);

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mLastPoints.at(0)));
    points.append(makeTouchPoint(mLastPoints.at(1)));
    return points;
}

QList<QPoint> SectorGesturePath::activePoints()
{
    QList<QPoint> points;
    points.append(mGestureLine.p1().toPoint());
    points.append(mGestureLine.p2().toPoint());    
    return points;
}

QList<TasTouchPoints> SectorGesturePath::pointAts(qreal value)
{
    mGestureLine.setAngle((mDistance*value)+mStartAngle);
    QList<QPoint> currentPoints = activePoints();

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(currentPoints.at(0), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(currentPoints.at(1), mLastPoints.at(1), mStartPoints.at(1)));
    mLastPoints = currentPoints;

    return points;
}

QList<TasTouchPoints> SectorGesturePath::endPoints()
{
    mGestureLine.setAngle(mDistance+mStartAngle);
    QList<QPoint> currentPoints = activePoints();

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(currentPoints.at(0), mLastPoints.at(0), mStartPoints.at(0)));
    points.append(makeTouchPoint(currentPoints.at(1), mLastPoints.at(1), mStartPoints.at(1)));
    mLastPoints = currentPoints;

    return points;
}

ArcsGesturePath::ArcsGesturePath(QLineF line1, QLineF line2, int distance)
{
    mGestureLine1 = line1;
    mGestureLine2 = line2;
    mStartAngle1 = mGestureLine1.angle();
    mStartAngle2 = mGestureLine2.angle();
    mDistance = distance;
}

QList<TasTouchPoints> ArcsGesturePath::startPoints()
{    
    mLastPoints = activePoints();
    mStartPoints = QList<QPoint>(mLastPoints);

    QList<TasTouchPoints> points;
    points.append(makeTouchPoint(mLastPoints.at(0)));
    points.append(makeTouchPoint(mLastPoints.at(1)));
    return points;
}

QList<QPoint> ArcsGesturePath::activePoints()
{
    QList<QPoint> points;
    points.append(mGestureLine1.p2().toPoint());
    points.append(mGestureLine2.p2().toPoint());
    return points;
}

QList<TasTouchPoints> ArcsGesturePath::pointAts(qreal value)
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

QList<TasTouchPoints> ArcsGesturePath::endPoints()
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
