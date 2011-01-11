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

#include "taslogger.h"
#include "tasgesturerecognizers.h"

const char* const DIST_ONE = "distance_1"; 
const char* const DIST_TWO = "distance_2"; 
const char* const DIFF     = "differential";
const char* const TYPE     = "type";
const char* const RADIUS   = "radius";
const char* const ROTATE_DIRECTION = "rotate_direction";

#ifdef TAS_MAEMO
#include <MApplication>
#include <MWindow>
#endif

/*!
    \class TasGestureRecognizer

    \brief Creates a TasGesture from the given data.
    
    TasGestures are created from the data passed from the testabilitydriver host.
    Recornizers are used to identify and create the correct gestures from the data.
*/
/*!
    \fn TasGesture* TasGestureRecognizer::create(TargetData data)

    Creates the TasGesture that matches the given data.

*/

QPoint TasGestureUtils::getPoint(TasCommand& command)
{
    int x = command.parameter("x").toInt();
    int y = command.parameter("y").toInt();
    return QPoint(x,y);
}

QPoint TasGestureUtils::getTargetPoint(TasCommand& command)
{
    QString targetId = command.parameter("targetId");
    QPoint targetPoint;
    if(command.parameter("targetType") == TYPE_GRAPHICS_VIEW){
        QGraphicsItem* targetItem = findGraphicsItem(targetId); 
        if(targetItem){
            viewPortAndPosition(targetItem, targetPoint);   
        }
    }
    else{
        QWidget* targetWidget = findWidget(targetId);
        if(targetWidget){
            targetPoint = targetWidget->mapToGlobal(targetWidget->rect().center());
        }
    }
    return targetPoint;
}


void TasGestureUtils::doTransform(QGraphicsItem* targetItem, QLineF& gestureLine)
{
    if(targetItem){
        QGraphicsView* view = TestabilityUtils::getViewForItem(targetItem);
        if(view && !view->viewportTransform().isIdentity()){                    
            QTransform transform = view->viewportTransform();
            QLineF transformedLine = transform.map(gestureLine);
            //set new angle and length based on transformation
            gestureLine.setLength(transformedLine.length());
            gestureLine.setAngle(transformedLine.angle());
        }    
    }
}

int TasGestureUtils::getDistance(TasCommand& command)
{
    return command.parameter("distance").toInt();
}





int TasGestureUtils::getDirection(TasCommand& command)
{
    int direction = command.parameter("direction").toInt();
#ifdef TAS_MAEMO
    // In meegotouch applications, modify the direction of the actual
    // angle of the device.
    MWindow *w = MApplication::activeWindow();
    if (w != 0 && w != NULL) {
        M::OrientationAngle angle = w->orientationAngle();
        switch(angle) {
        case M::Angle90:
            direction += 90;
            break;
        case M::Angle180:
            direction += 180;
            break;
        case M::Angle270:
            direction += 270;
            break;
        case M::Angle0:
        default:
            break;
        }
    }
#endif

    direction -= 90;
    direction = direction * -1;
    return direction;

}

QLineF TasGestureUtils::makeLine(QPoint start, int length, int angle)
{
    QLineF line;
    line.setP1(start);
    line.setLength(length);
    line.setAngle(angle);
    return line;
}


/*!
    \class LineTasGestureRecognizer

    \brief Creates a LineTasGesture from the given data.

    Creates a linear single touch gesture from the given data.
*/

LineTasGestureRecognizer::LineTasGestureRecognizer()
{
    mTypes << "MouseGesture" << "MouseGestureTo" << "MouseGestureToCoordinates" << "MouseGestureFromCoordinates";
}


/*!
    \fn TasGesture* bool TasGestureRecognizer::isSupportedType(const QString& gestureType)

    Return true if the given gesture type can be recognined and created by the recognizer.
*/
bool LineTasGestureRecognizer::isSupportedType(const QString& gestureType)
{
    return mTypes.contains(gestureType);
}


TasGesture* LineTasGestureRecognizer::create(TargetData data)
{
    TasCommand& command = *data.command;
    QPoint point = data.targetPoint;

    QLineF gestureLine;
    if(command.name() == "MouseGestureFromCoordinates"){
        QPoint start = mUtils.getPoint(command);
        gestureLine.setP1(QPointF(start));    
    }
    else{
        gestureLine.setP1(QPointF(point));
    }
 
    if(command.name() == "MouseGesture" || command.name() == "MouseGestureFromCoordinates"){
        gestureLine.setLength(mUtils.getDistance(command));
        gestureLine.setAngle(mUtils.getDirection(command));
        mUtils.doTransform(data.targetItem, gestureLine);
    }
    else{
        QPoint end;
        if(command.name() == "MouseGestureTo"){
            end = mUtils.getTargetPoint(command);
        }
        else{
            end = mUtils.getPoint(command);
        }
        gestureLine.setP1(QPointF(point));
        gestureLine.setP2(QPointF(end));
    }
    return new LineTasGesture(data, gestureLine);
}


PointsTasGestureRecognizer::PointsTasGestureRecognizer()
{
}

bool PointsTasGestureRecognizer::isSupportedType(const QString& gestureType)
{
    return gestureType == "MouseGesturePoints";
}

TasGesture* PointsTasGestureRecognizer::create(TargetData data)
{
    TasCommand& command = *data.command;
    QWidget* target = data.target;
    
    QList<QPoint> gesturePoints;
    QList<int> gestureIntervals;
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
    PointsTasGesture* gesture = new PointsTasGesture(data, gesturePoints);
    gesture->setIntervals(gestureIntervals);
    return gesture;
}


PinchZoomTasGestureRecognizer::PinchZoomTasGestureRecognizer()
{}

bool PinchZoomTasGestureRecognizer::isSupportedType(const QString& gestureType)
{
    return gestureType == "PinchZoom";
}

TasGesture* PinchZoomTasGestureRecognizer::create(TargetData data)
{
    TasCommand& command = *data.command;

    if(!validateZoomParams(command)){
        return 0;
    }

    QPoint point = data.targetPoint;
    if(command.parameter("useCoordinates") == "true"){ 
        point = mUtils.getPoint(command);
    }

    int distance_1 = command.parameter(DIST_ONE).toInt();
    int distance_2 = command.parameter(DIST_TWO).toInt();
    int differential = command.parameter(DIFF).toInt();        
    QLineF line1;
    QLineF line2;
    QPoint start1 = point;
    QPoint start2 = point;
    if(differential > 1){
        QLineF line = mUtils.makeLine(point,differential/2, mUtils.getDirection(command));
        start1 = line.p2().toPoint();
        line.setAngle(mUtils.getDirection(command)+180);
        start2 = line.p2().toPoint();
    }
    line1 = mUtils.makeLine(start1, distance_1, mUtils.getDirection(command));
    line2 = mUtils.makeLine(start2, distance_2, mUtils.getDirection(command)+180);

    //this is the diff line
    if(command.parameter(TYPE) == "in"){
        return new PinchZoomTasGesture(data, line1, line2);
    }
    else{
        //flip the lines if zoom out
        return new PinchZoomTasGesture(data, QLineF(line1.p2(), line1.p1()), QLineF(line2.p2(), line2.p1()));
    }

}

bool PinchZoomTasGestureRecognizer::validateZoomParams(TasCommand& command)
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

RotationTasGestureRecognizer::RotationTasGestureRecognizer()
{}

bool RotationTasGestureRecognizer::isSupportedType(const QString& gestureType)
{
    return gestureType == "Rotate";
}

TasGesture* RotationTasGestureRecognizer::create(TargetData data)
{
    TasCommand& command = *data.command;

    if(!validateRotationParams(command)){
        return false;
    }

    QPoint point = data.targetPoint;
    if(command.parameter("useCoordinates") == "true"){ 
        point = mUtils.getPoint(command);
        TasLogger::logger()->debug("set point");
    }

    int radius = command.parameter(RADIUS).toInt();
    int distance = mUtils.getDistance(command);
    int direction = mUtils.getDirection(command);

    if(command.parameter(ROTATE_DIRECTION) == "Clockwise"){
        distance = distance * -1;
    }

    QLineF line = mUtils.makeLine(point, radius, direction);

    if(command.parameter(TYPE) == "one_point"){
        return new SectorTasGesture(data, line, distance);
    }
    else{
        QLineF line2 = mUtils.makeLine(point, radius, direction+180);
        return new ArcsTasGesture(data, line, line2, distance);
    }
}

bool RotationTasGestureRecognizer::validateRotationParams(TasCommand& command)
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


