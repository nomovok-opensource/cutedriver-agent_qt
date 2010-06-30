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
 


#include <QtTest/qtestspontaneevent.h>

#include "taslogger.h"
#include "mousehandler.h"

#include <tasdeviceutils.h>

/*!
  \class MouseHandler
  \brief MouseHandler generates mouse press and release events.

*/    


MouseHandler::MouseHandler()
{
    mTouchPointCounter = 0;
    mUseTapScreen = false;
}

MouseHandler::~MouseHandler()
{
    QHashIterator<int, QList<int>*> i(mTouchIds);
    while (i.hasNext()) {
        delete i.next();
    }
    mTouchIds.clear();
}
  
bool MouseHandler::executeInteraction(TargetData data)
{
    //TasLogger::logger()->debug("MouseHandler::executeInteraction");
    bool wasConsumed = false;

    TasCommand& command = *data.command;
    QWidget* target = data.target;
    QPoint point = data.targetPoint;
    QGraphicsItem* targetItem = data.targetItem;

    QString commandName = command.name();

    mUseTapScreen = command.parameter("useTapScreen") == "true";
    if (!mUseTapScreen) {
        TasLogger::logger()->debug("MouseHandler::executeInteraction not using tap_screen: " + command.name());
    } else {
        TasLogger::logger()->debug("MouseHandler::executeInteraction USING XEVENTS: " + command.name());        
    }

    if(commandName == "MousePress" || commandName == "MouseRelease" || commandName == "MouseClick" || 
       commandName == "Tap" || commandName == "TapScreen" || commandName == "MouseMove" || 
       commandName == "MouseDblClick"){
        
        Qt::MouseButton button = getMouseButton(command);
        setPoint(point, command);
        checkMoveMouse(command, point);
        wasConsumed = true;
        
        if (commandName == "MouseClick" || commandName == "Tap" || commandName == "TapScreen"){
            int count = 1;
            if(!command.parameter("count").isEmpty()){
                count = command.parameter("count").toInt();       
            }
            if(commandName == "TapScreen"){
                target = qApp->widgetAt(point.x(), point.y());
            }            
            if(target){
//                int duration = 1000000*command.parameter("duration").toFloat();


                if(command.parameter("interval").isEmpty()){
                    for(int i = 0 ; i < count; i++){
                        doMousePress(target, targetItem, button, point);
                        doMouseRelease(target, targetItem, button, point);
                     }                    
                }
                else{
                    //interval based tapping
                    int interval = command.parameter("interval").toInt();
                    new Tapper(this, count, interval, target, targetItem, button, point);
                }

            }
        }
        else if(commandName == "MouseDblClick"){
            doMouseDblClick(target, button, point);
        }   
        else if(commandName == "MousePress"){
            doMousePress(target, targetItem, button, point);
        }
        else if(commandName == "MouseRelease"){
            doMouseRelease(target, targetItem, button, point);
        }
        else if (commandName == "MouseMove"){                        
            doMouseMove(target, targetItem, point, button);
        }
    }
    else if (commandName == "Scroll"){
        wasConsumed = true;

        Qt::MouseButton button = getMouseButton(command);
        int deltaValue = 0 ;
        Qt::Orientation direction = Qt::Vertical;
                   
        if (!command.parameter("delta").isEmpty()) {
            deltaValue = command.parameter("delta").toInt();
        }        
        if (!command.parameter("orientation").isEmpty()){
            if ( command.parameter("orientation") == ":horizontal"){
                direction = Qt::Horizontal;
            }
        }        
        doScroll(target, point, deltaValue, button, direction);                
    }
    return wasConsumed;
}

Qt::MouseButton MouseHandler::getMouseButton(TasCommand& command)
{
    Qt::MouseButton btn = Qt::NoButton;
    Qt::MouseButtons buttons = qApp->mouseButtons();
    QString button = command.parameter("button");
    if (!button.isEmpty()){         
        btn = static_cast<Qt::MouseButton>(button.toInt());
    }        
    else if(buttons & Qt::LeftButton){
        btn = Qt::LeftButton;
    }
    else if(buttons & Qt::RightButton){
        btn = Qt::RightButton;
    }
    return btn;
}

void MouseHandler::checkMoveMouse(TasCommand& command, QPoint point)
{
    if(command.parameter("mouseMove") == "true"){                
        moveCursor(point);
    }           
}

/*!
 Set point from parameters. 
 */
void MouseHandler::setPoint(QPoint& point, TasCommand& command)
{
    if(command.parameter("useCoordinates") == "true"){ 
        int x = command.parameter("x").toInt();
        int y = command.parameter("y").toInt();
        point.setX(x);
        point.setY(y);           
    }
}

/*!
    Send a mouseEvent to qApp to the given target widget. 
*/
void MouseHandler::doMousePress(QWidget* target, QGraphicsItem* targetItem, Qt::MouseButton button, QPoint point)
{ 
    if (target) {
        target->setFocus(Qt::MouseFocusReason);
    }
    
    if(acceptsTouchEvent(target, targetItem)){
        doTouchBegin(target, targetItem, toTouchPoints(point));
    }
    else{
        QMouseEvent* eventPress = new QMouseEvent(QEvent::MouseButtonPress, target->mapFromGlobal(point), point, button, button, 0);    
        sendMouseEvent(target, eventPress);
    }
}

void MouseHandler::doTouchBegin(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, targetItem, Qt::TouchPointPressed, points);
    QTouchEvent* touchPress = new QTouchEvent(QEvent::TouchBegin, QTouchEvent::TouchScreen, Qt::NoModifier, Qt::TouchPointPressed, touchPoints);
    touchPress->setWidget(target);
    sendTouchEvent(target, touchPress);
}

/*!
    Send a mouseEvent to qApp to the given target widget. 
*/
void MouseHandler::doMouseRelease(QWidget* target, QGraphicsItem* targetItem, Qt::MouseButton button, QPoint point)
{        
    if(acceptsTouchEvent(target, targetItem)){
        doTouchEnd(target, targetItem, toTouchPoints(point));
    }
    else{
        QMouseEvent* eventRelease = new QMouseEvent(QEvent::MouseButtonRelease, target->mapFromGlobal(point), point, button, Qt::NoButton, 0);    
        sendMouseEvent(target, eventRelease);
    }

}

QList<TasTouchPoints> MouseHandler::toTouchPoints(QPoint point)
{
    QList<TasTouchPoints> points;
    TasTouchPoints touchPoint;
    touchPoint.screenPoint = point;
    points.append(touchPoint);
    return points;
}
void MouseHandler::doTouchEnd(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, targetItem, Qt::TouchPointReleased, points);
    QTouchEvent *touchRelease = new QTouchEvent(QEvent::TouchEnd, QTouchEvent::TouchScreen, Qt::NoModifier, Qt::TouchPointReleased, touchPoints);
    touchRelease->setWidget(target);
    sendTouchEvent(target, touchRelease);
}

void MouseHandler::doMouseDblClick(QWidget* target, Qt::MouseButton button, QPoint point)
{
    QMouseEvent* eventDblClick = new QMouseEvent(QEvent::MouseButtonDblClick, target->mapFromGlobal(point), point, button, Qt::NoButton, 0);    
    sendMouseEvent(target, eventDblClick);
}

/*!
    Move cursor to given position.
*/
void MouseHandler::doMouseMove(QWidget* target, QGraphicsItem* targetItem, QPoint point, Qt::MouseButton button)
{ 
    moveCursor(point);
    if(acceptsTouchEvent(target, targetItem) && button != Qt::NoButton){
        doTouchUpdate(target, targetItem, toTouchPoints(point));
    }
    else{
        QMouseEvent* eventMove = new QMouseEvent(QEvent::MouseMove, target->mapFromGlobal(point), point, button, button, 0);
        sendMouseEvent(target, eventMove);
    }
}

bool MouseHandler::acceptsTouchEvent(QWidget* target, QGraphicsItem* targetItem)
{
    bool accepts = false;
    if(target->testAttribute(Qt::WA_AcceptTouchEvents)){
        accepts = true;
    } 
    //the viewport seems to accept but the item not
    if(targetItem && !targetItem->acceptTouchEvents()){
        accepts = false;
    }
    //accepts = false;
    return accepts;
}
void MouseHandler::doTouchUpdate(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points)
{
    QList<QTouchEvent::TouchPoint> touchPoints = convertToTouchPoints(target, targetItem, Qt::TouchPointMoved, points);
    QTouchEvent* touchMove = new QTouchEvent(QEvent::TouchUpdate, QTouchEvent::TouchScreen, Qt::NoModifier, Qt::TouchPointMoved, touchPoints);
    touchMove->setWidget(target);
    sendTouchEvent(target, touchMove);
}

void MouseHandler::moveCursor(QPoint point)
{
//    TasLogger::logger()->debug("MouseHandler::moveCursor");
    if (mUseTapScreen) {
        TasLogger::logger()->debug("MouseHandler::moveCursor using XEVENTS");
        TasDeviceUtils::sendMouseEvent(point.x(), 
                                       point.y(), 
                                       Qt::NoButton, 
                                       QEvent::MouseMove);
                
    } else {
        QCursor::setPos(point);
        qApp->processEvents();
    }
}


/*!
    Scroll the mouse wheel 
*/
void MouseHandler::doScroll(QWidget* target, QPoint& point, int delta, Qt::MouseButton button,  Qt::Orientation orient)
{
    QWheelEvent* event = new QWheelEvent(point, target->mapToGlobal(point), delta, button, 0, orient);
    qApp->postEvent(target, event);
}

void MouseHandler::sendMouseEvent(QWidget* target, QMouseEvent* event)
{    
    if(mUseTapScreen){
        TasLogger::logger()->debug("MouseHandler::sendMousEvent using XEVENTS");
        TasDeviceUtils::sendMouseEvent(event->globalX(), 
                                       event->globalY(), 
                                       event->button(), 
                                       event->type());

    } else {
        QSpontaneKeyEvent::setSpontaneous(event);
        qApp->postEvent(target, event);   
    }
}

void MouseHandler::sendTouchEvent(QWidget* target, QTouchEvent* event)
{
    QSpontaneKeyEvent::setSpontaneous(event);
    qApp->postEvent(target, event);   
}

QList<QTouchEvent::TouchPoint> MouseHandler::convertToTouchPoints(QWidget* target, QGraphicsItem* targetItem, Qt::TouchPointState state,
                                                                  QList<TasTouchPoints> points)
{
    bool remove = false;
    QList<int> *pointIds;
    //we need to store the touchpoint ids, the same id must be attached for untill touch point released
    if(targetItem) {
        int itemId = (int)targetItem;
        if(state == Qt::TouchPointReleased && mTouchIds.contains(itemId)){
            pointIds = mTouchIds.take(itemId);
            remove = true;
        }
        else if(state == Qt::TouchPointMoved && mTouchIds.contains(itemId)){
            pointIds = mTouchIds.value(itemId);
        }
        else{
            pointIds = new QList<int>();
            mTouchIds.insert(itemId, pointIds);
        }
    }
    else{
        pointIds = new QList<int>();
        remove = true;
    }

    QList<QTouchEvent::TouchPoint> touchPoints;    
    if(!points.isEmpty()){
        for(int i = 0 ; i < points.size() ; i++){
            if(pointIds->size() <= i ){
                mTouchPointCounter++;
                pointIds->append(mTouchPointCounter);                
            }
            touchPoints.append(makeTouchPoint(target, targetItem, points.at(i), state, pointIds->at(i), false));
        }
    }

    if(remove) delete pointIds;

    return touchPoints;
}

QTouchEvent::TouchPoint MouseHandler::makeTouchPoint(QWidget* target, QGraphicsItem* targetItem,
                                                     TasTouchPoints points, Qt::TouchPointState state,
                                                     int id, bool primary)
{
    Q_UNUSED(targetItem);
    QTouchEvent::TouchPoint touchPoint(id);
    Qt::TouchPointStates states = state;
    if(primary){
        states |= Qt::TouchPointPrimary;
    }
    touchPoint.setState(states);
    touchPoint.setPos(target->mapFromGlobal(points.screenPoint));    
    touchPoint.setScreenPos(points.screenPoint);    
    QRect screenGeometry = QApplication::desktop()->screenGeometry(points.screenPoint);
    touchPoint.setNormalizedPos(QPointF(points.screenPoint.x() / screenGeometry.width(),
                                        points.screenPoint.y() / screenGeometry.height()));

    //in addition to the position we also need to set last and start positions as 
    //some gesture may depend on them
    if(!points.lastScreenPoint.isNull()){
        touchPoint.setLastPos(target->mapFromGlobal(points.lastScreenPoint));    
        touchPoint.setLastScreenPos(points.lastScreenPoint);    
        touchPoint.setLastNormalizedPos(QPointF(points.lastScreenPoint.x() / screenGeometry.width(),
                                                points.lastScreenPoint.y() / screenGeometry.height()));
    }
    if(!points.startScreenPoint.isNull()){
        touchPoint.setStartPos(target->mapFromGlobal(points.startScreenPoint));    
        touchPoint.setStartScreenPos(points.startScreenPoint);    
        touchPoint.setStartNormalizedPos(QPointF(points.startScreenPoint.x() / screenGeometry.width(),
                                                points.startScreenPoint.y() / screenGeometry.height()));
    }
    return touchPoint;
}


Tapper::Tapper(MouseHandler* handler, int count, int interval, QWidget* target, 
               QGraphicsItem* targetItem, Qt::MouseButton button, QPoint point)
{
    mMaxCount = count;
    mTapCount = 0;
    mHandler = handler;
    mTarget = target;
    mTargetItem = targetItem;
    mButton = button;
    mPoint = point;
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(tap()));
    mTimer.start(interval);
}

void Tapper::tap()
{
    TasLogger::logger()->debug("Tapper::tap");
    mTapCount++;
    
    mHandler->doMousePress(mTarget, mTargetItem, mButton, mPoint);
    mHandler->doMouseRelease(mTarget, mTargetItem, mButton, mPoint);

    if( mTapCount >= mMaxCount ){
        mTimer.stop();        
        deleteLater();
    }
}
