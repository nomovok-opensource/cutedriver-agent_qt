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


GestureHandler::GestureHandler(QObject* parent)
    :QObject(parent)
{
    mGesture = 0;
}

GestureHandler::~GestureHandler()
{    
    if(mGesture){
        delete mGesture;
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

    TasCommand& command = *data.command;
    mTargetDetails = getParameters(command);
    mTargetDetails.widget = data.target;
    mTargetDetails.item = data.targetItem;

    QString commandName = command.name();
//     QPoint point = data.targetPoint;
//     setPoint(point, command);

    if(mGesture){
        delete mGesture;
        mGesture = 0;
    }

    if(commandName == "MouseGesturePoints"){
        TasCommand& command = *data.command;
        QPoint point = data.targetPoint;
        QWidget* target = data.target;
        //        QGraphicsItem* targetItem = data.targetItem;

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
        mGesture = new PointsTasGesture(gesturePoints);
        //   mGesture->setIntervals(gestureIntervals);
        startGesture();
        wasConsumed = true;
    }
    else if(commandName.startsWith("MouseGesture")){
        QLineF gestureLine = makeGestureLine(data);
        if(!gestureLine.isNull()){
            mGesture = new LineTasGesture(gestureLine);
            startGesture();
        }
        else{
            TasLogger::logger()->error("GestureHandler::executeInteraction invalid line. Cannot start gesture.");
        }
        wasConsumed = true;
    }
    return wasConsumed;
}

/*!
 Makes a QLineF to be used as the gesture from the given data. Check 
 isNull before using incase given data is not valid.
 */
QLineF GestureHandler::makeGestureLine(TargetData data)
{
    TasCommand& command = *data.command;
    QString commandName = command.name();

    QPoint point = data.targetPoint;
    QGraphicsItem* targetItem = data.targetItem;
    TargetDetails targetDetails = getParameters(command);
    QLineF gestureLine;
    setPoint(point, command);        

    if (commandName == "MouseGesture"){  
        gestureLine.setP1(QPointF(point));
        gestureLine.setLength(targetDetails.distance);
        gestureLine.setAngle(targetDetails.direction);
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
    else if(commandName == "MouseGestureTo"){     
        QPoint end = getTargetPoint(targetDetails);
        if(!end.isNull()){
            gestureLine.setP1(QPointF(point));
            gestureLine.setP2(QPointF(end));
        }
    }
    else if(commandName == "MouseGestureToCoordinates"){     
        int x = command.parameter("x").toInt();
        int y = command.parameter("y").toInt();
        QPoint end(x,y);
        if(!end.isNull()){
            gestureLine.setP1(QPointF(point));
            gestureLine.setP2(QPointF(end));
        }
    }
    else if(commandName == "MouseGestureFromCoordinates"){     
        int x = command.parameter("x").toInt();
        int y = command.parameter("y").toInt();        
        QPoint start(x,y);
        if(!start.isNull()){
            gestureLine.setP1(QPointF(start));
            gestureLine.setLength(targetDetails.distance);
            gestureLine.setAngle(targetDetails.direction);
        }
    }
    return gestureLine;
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
    mTimeLine->setDuration(mTargetDetails.duration);
    mTimeLine->setFrameRange(0, mTargetDetails.duration/FRAME_RANGE_DIV);    
    mTimeLine->start();
}


/*!
    Move the pointer based on the direction and stepsize.
*/
void GestureHandler::timerEvent(qreal value)
{
    //do not send an event if no movement
    if(mPrevious != mGesture->pointsAt(value).at(0).screenPoint){
        doMouseMove(mTargetDetails.widget, mTargetDetails.item, mGesture->pointsAt(value).at(0).screenPoint, mTargetDetails.button);        
    }
    mPrevious = mGesture->pointsAt(value).at(0).screenPoint;
}

void GestureHandler::finished()
{
    //make sure the end is reached
    //do not send an event if no movement
    if(mPrevious != mGesture->endPoints().at(0).screenPoint){
        doMouseMove(mTargetDetails.widget, mTargetDetails.item, mGesture->endPoints().at(0).screenPoint, mTargetDetails.button);
    }
    if(mTargetDetails.release){
        if(mTargetDetails.isDrag){
            //send move event to the end point with intention to cause a stopping effect
            doMouseMove(mTargetDetails.widget, mTargetDetails.item, mGesture->endPoints().at(0).screenPoint, mTargetDetails.button);          
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
    if(mTargetDetails.press){
        doMouseMove(mTargetDetails.widget, mTargetDetails.item, mGesture->startPoints().at(0).screenPoint, Qt::NoButton);
        mPrevious = mGesture->startPoints().at(0).screenPoint;
        doMousePress(mTargetDetails.widget, mTargetDetails.item, mTargetDetails.button, mGesture->startPoints().at(0).screenPoint);            
    }

}

void GestureHandler::releaseMouse()
{
    doMouseRelease(mTargetDetails.widget, mTargetDetails.item, mTargetDetails.button, mGesture->endPoints().at(0).screenPoint);            
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

TargetDetails GestureHandler::getParameters(TasCommand& command)
{
    TargetDetails targetDetails;
    targetDetails.widget = 0;
    targetDetails.item = 0;

    if (!command.parameter("direction").isEmpty()) {
        int direction = command.parameter("direction").toInt();
        direction -= 90;
        direction = direction * -1;
        targetDetails.direction = direction;
    }
    if (!command.parameter("distance").isEmpty()) {
        targetDetails.distance = command.parameter("distance").toInt();
    }    
    if (!command.parameter("speed").isEmpty()) {
        targetDetails.duration = command.parameter("speed").toInt();
    }
    if (!command.parameter("targetId").isEmpty()) {
        targetDetails.targetId = command.parameter("targetId");
    }
    if (!command.parameter("targetType").isEmpty()) {
        targetDetails.targetType = command.parameter("targetType");
    }

    targetDetails.isDrag = false;
    if (command.parameter("isDrag") == "true") {
        targetDetails.isDrag = true;
    }

    targetDetails.button = getMouseButton(command);
    
    targetDetails.press = true;
    targetDetails.release = true;
    if (command.parameter("press") == "false") {
        targetDetails.press = false;
    }
    if (command.parameter("release") == "false") {
        targetDetails.release = false;
    }
    return targetDetails;
}

QPoint GestureHandler::getTargetPoint(TargetDetails details)
{
    QPoint targetPoint;
    if(details.targetType == TYPE_GRAPHICS_VIEW){
        QGraphicsItem* targetItem = findGraphicsItem(details.targetId); 
        if(targetItem){
            viewPortAndPosition(targetItem, targetPoint);   
        }
    }
    else{
        QWidget* targetWidget = findWidget(mTargetDetails.targetId);
        if(targetWidget){
            targetPoint = targetWidget->mapToGlobal(targetWidget->rect().center());
        }
    }
    return targetPoint;
}
