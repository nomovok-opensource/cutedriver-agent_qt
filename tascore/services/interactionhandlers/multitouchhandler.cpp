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
    mGesture = 0;
}

MultitouchHandler::~MultitouchHandler()
{    
    if(mGesture){
        delete mGesture;
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
    mTargetDetails = getParameters(command);
    mTargetDetails.widget = data.target;
    mTargetDetails.item = data.targetItem;

    if(commandName == "PinchZoom" || commandName == "Rotate"){
        //from mousehandler
        setPoint(point, command);
        //from gesturehandler
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
        QList<QLineF> lines;
        TargetData targetData;
        foreach(targetData, dataList){
            if(targetData.command->name() == "MouseClick" || targetData.command->name() == "MousePress" ||
               targetData.command->name() == "Tap" ){            
                touchPoints.append(convertToTouchPoints(targetData.target,targetData.targetItem,
                                                        Qt::TouchPointPressed, toTouchPoints(targetData.targetPoint, false)));
            }
            if(targetData.command->name() == "MouseClick" || targetData.command->name() == "MouseRelease" ||
               targetData.command->name() == "Tap" ){            
                touchReleasePoints.append(convertToTouchPoints(targetData.target,targetData.targetItem,
                                                               Qt::TouchPointReleased, toTouchPoints(targetData.targetPoint, false)));
            }            
            //points gesture not supported
            if(targetData.command->name().startsWith("MouseGesture") && targetData.command->name() != "MouseGesturePoints"){
                QLineF gestureLine =  makeGestureLine(targetData);
                if(!gestureLine.isNull()){
                    lines.append(gestureLine);
                }
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

        if(!lines.isEmpty()){
            if(mGesture){
                delete mGesture;
                mGesture = 0;
            }   
            mGesture = new MultiLineTasGesture(lines);
            startGesture();
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
    QLineF line = makeLine(point,differential/2, mTargetDetails.direction);

    //actual gesture lines
    QLineF line1 = makeLine(line.p2().toPoint(), distance_1, line.angle());
    line.setAngle(mTargetDetails.direction+180);
    QLineF line2 = makeLine(line.p2().toPoint(), distance_2, line.angle());

    if(mGesture){
        delete mGesture;
        mGesture = 0;
    }
    if(command.parameter(TYPE) == "in"){
        mGesture = new PinchZoomTasGesture(line1, line2);
    }
    else if(command.parameter(TYPE) == "out"){
        //flip the lines if zoom out
        mGesture = new PinchZoomTasGesture(QLineF(line1.p2(), line1.p1()), QLineF(line2.p2(), line2.p1()));
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
        mTargetDetails.distance = mTargetDetails.distance * -1;
    }

    QLineF line = makeLine(point, radius, mTargetDetails.direction);
    if(command.parameter(TYPE) == "one_point"){
        mGesture = new SectorTasGesture(line, mTargetDetails.distance);
    }
    else if(command.parameter(TYPE) == "two_point"){
        QLineF line2 = makeLine(point, radius, mTargetDetails.direction+180);
        mGesture = new ArcsTasGesture(line, line2, mTargetDetails.distance);
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
    doTouchUpdate(mTargetDetails.widget, mTargetDetails.item, mGesture->pointsAt(value));
}

void MultitouchHandler::finished()
{
    //make sure the end is reached
    doTouchUpdate(mTargetDetails.widget, mTargetDetails.item, mGesture->endPoints());
    doTouchEnd(mTargetDetails.widget, mTargetDetails.item, mGesture->endPoints());
}

void MultitouchHandler::beginGesture()
{
    doTouchBegin(mTargetDetails.widget, mTargetDetails.item, mGesture->startPoints());
}

