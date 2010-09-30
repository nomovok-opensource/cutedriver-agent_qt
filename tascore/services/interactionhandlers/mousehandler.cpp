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

#include <QToolBar>
#include <QToolButton>
#include <QMenuBar>
#include <QMenu>

/*!
  \class MouseHandler
  \brief MouseHandler generates mouse press and release events.

*/


MouseHandler::MouseHandler()
{
    mCommands << "MousePress" << "MouseRelease" << "MouseClick"
              << "Tap" << "TapScreen" << "MouseMove" <<  "MouseDblClick";
}


MouseHandler::~MouseHandler()
{
}


bool MouseHandler::executeInteraction(TargetData data)
{
    //TasLogger::logger()->debug("MouseHandler::executeInteraction");
    bool wasConsumed = false;
    TasCommand& command = *data.command;
    QString commandName = command.name();
    if(mCommands.contains(commandName)){
        TapDetails details = makeDetails(data);
        setPoint(command, details);
        wasConsumed = true;
        checkMoveMouse(details);

        if (commandName == "MouseClick" || commandName == "Tap" || commandName == "TapScreen"){
            int count = 1;
            if(!command.parameter("count").isEmpty()){
                count = command.parameter("count").toInt();
            }
            if(commandName == "TapScreen"){
                details.target = qApp->widgetAt(details.point.x(), details.point.y());
            }
            if(details.target){
                int duration = command.parameter("duration").toFloat();

                if(command.parameter("interval").isEmpty()){
                    for(int i = 0 ; i < count; i++){
                        press(details);
                        if (duration != 0) {
                            TasCoreUtils::wait(duration);
                        }
                        release(details);
                     }
                }
                else{
                    //interval based tapping
                    int interval = command.parameter("interval").toInt();
                    new Tapper(this, details, count, interval);
                }

            }
        }
        else if(commandName == "MouseDblClick"){
            mMouseGen.doMouseDblClick(details.target, details.button, details.point);
        }
        else if(commandName == "MousePress"){
            press(details);
        }
        else if(commandName == "MouseRelease"){
            release(details);
        }
        else if (commandName == "MouseMove"){
            move(details);
        }
    }
    else if (commandName == "Scroll"){
        wasConsumed = true;
        TapDetails details = makeDetails(data);

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
        mMouseGen.doScroll(details.target, details.point, deltaValue, details.button, direction);
    }
    else if(commandName == "Hover" || commandName == "Trigger"){
        wasConsumed = performActionEvent(makeDetails(data));
    }
    return wasConsumed;
}


static bool visibleWidgetGlobalPoint(QWidget *widget, QPoint &resultPoint)
{
    if (widget && widget->isVisible()) {
        QRegion visibleRegion = widget->visibleRegion();
        if (!visibleRegion.isEmpty()) {
            resultPoint = widget->mapToGlobal(visibleRegion.rects().at(0).center());
            return true;
        }
    }
    return false;
}

static bool visibleWidgetGlobalPoint(QWidget *widget, const QRect &extraClipRect, QPoint &resultPoint)
{

    if (widget && widget->isVisible()) {
        QRegion clippedVisibleRegion = widget->visibleRegion().intersected(extraClipRect);
        if (!clippedVisibleRegion.isEmpty()) {
            resultPoint = widget->mapToGlobal(clippedVisibleRegion.rects().at(0).center());
            return true;
        }
    }
    return false;
}


bool MouseHandler::performActionEvent(TapDetails details)
{
    bool stateOk;

    int id = details.command->parameter("id").toInt(&stateOk);
    QAction* action = stateOk
            ? getAction(details.target, id)
            : NULL;

    stateOk = (action != NULL);

    if(stateOk) {
        //only way to activate the action is to simulate mouse events
        //attempt to get the coordinates for the action...
        if(QMenuBar* bar = qobject_cast<QMenuBar*>(details.target)){
            //TasLogger::logger()->debug(QString("%1 got QMenuBar").arg(__PRETTY_FUNCTION__));
            stateOk = visibleWidgetGlobalPoint(details.target, bar->actionGeometry(action), details.point);
        }
        else if(QMenu* menu = qobject_cast<QMenu*>(details.target)){
            //TasLogger::logger()->debug(QString("%1 got QMenu").arg(__PRETTY_FUNCTION__));
            stateOk = visibleWidgetGlobalPoint(details.target, menu->actionGeometry(action), details.point);
        }
        else if(QToolBar* bar = qobject_cast<QToolBar*>(details.target)){
            //TasLogger::logger()->debug(QString("%1 got QToolBar").arg(__PRETTY_FUNCTION__));
            details.target = bar->widgetForAction(action);
            stateOk = visibleWidgetGlobalPoint(details.target, details.point);
        }
        else if(qobject_cast<QToolButton*>(details.target)){
            //TasLogger::logger()->debug(QString("%1 got QToolButton").arg(__PRETTY_FUNCTION__));
            stateOk = visibleWidgetGlobalPoint(details.target, details.point);
        }
        else{
            //skip since we are not sure what to do...
            TasLogger::logger()->info(QString("%1 error: class %2 unsupported")
                                      .arg(__PRETTY_FUNCTION__)
                                      .arg(details.target->metaObject()->className()));
            stateOk = false;
        }
    }

    if (stateOk) {
#if 0
        TasLogger::logger()->debug(QString("%1 MIDDLE1: %2 for class %3, global point (%4,%5)")
                                   .arg(__PRETTY_FUNCTION__)
                                   .arg(details.command->name())
                                   .arg(details.target->metaObject()->className())
                                   .arg(details.point.x()).arg(details.point.y()));
        TasLogger::logger()->debug(QString("%1 MIDDLE2: pointertype %2 button %3 extra '%4'")
                                   .arg(__PRETTY_FUNCTION__)
                                   .arg(details.pointerType)
                                   .arg(details.button)
                                   .arg(details.extraIdentifier));
#endif
        //add mouse events tha match the action type
        if ( details.command->name() == "Hover"){
            details.button = Qt::NoButton;
            move(details);
        }
        else{
            details.button = Qt::LeftButton;
            press(details);
            release(details);
        }
    }

    return stateOk;
}

/*!
    Get the correct action associated with the widget and calculate its center pos.
 */
QAction* MouseHandler::getAction(QWidget* widget, int id)
{
    QAction* action = NULL;
    QList<QAction*> actions = widget->actions();
    if(actions.size() > 0){
        QAction* target = (QAction*)id;
        for(int i = 0 ; i < actions.size(); i++){
            QAction * ac = actions.at(i);
            if ( ac == target){
                action = ac;
                break;
            }
        }
    }
    return action;
}

MouseHandler::TapDetails MouseHandler::makeDetails(TargetData data)
{
    TasCommand& command = *data.command;
    TapDetails details;
    details.command = data.command;
    details.target = data.target;
    details.point = data.targetPoint;
    details.targetItem = data.targetItem;
    details.button = getMouseButton(command);
    details.pointerType = MouseHandler::TypeMouse;
    if(!command.parameter(POINTER_TYPE).isEmpty()){
        details.pointerType = static_cast<MouseHandler::PointerType>(command.parameter(POINTER_TYPE).toInt());
    }
    mMouseGen.setUseTapScreen(command.parameter("useTapScreen") == "true");

    return details;
}

void MouseHandler::press(TapDetails details)
{
    if(details.pointerType == TypeMouse || details.pointerType == TypeBoth){
        mMouseGen.doMousePress(details.target, details.button, details.point);
    }
    if(details.pointerType == TypeTouch || details.pointerType == TypeBoth){
        //set primary only when mouse events are sent
        bool primary  = (details.pointerType == TypeBoth);
        mTouchGen.doTouchBegin(details.target, details.targetItem, details.point, primary, details.extraIdentifier);
    }
}

void MouseHandler::move(TapDetails details)
{
    if(details.pointerType == TypeMouse || details.pointerType == TypeBoth){
        mMouseGen.doMouseMove(details.target, details.point, details.button);
    }
    if(details.pointerType == TypeTouch || details.pointerType == TypeBoth){
        bool primary  = (details.pointerType == TypeBoth);
        mTouchGen.doTouchUpdate(details.target, details.targetItem, details.point, primary);
    }
}
void MouseHandler::release(TapDetails details)
{
    if(details.pointerType == TypeMouse || details.pointerType == TypeBoth){
        mMouseGen.doMouseRelease(details.target, details.button, details.point);
    }
    if(details.pointerType == TypeTouch || details.pointerType == TypeBoth){
        bool primary  = (details.pointerType == TypeBoth);
        mTouchGen.doTouchEnd(details.target, details.targetItem, details.point, primary, details.extraIdentifier);
    }
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

void MouseHandler::checkMoveMouse(TapDetails details)
{
    if(details.pointerType == TypeMouse || details.pointerType == TypeBoth){
        if(details.command->parameter("mouseMove") == "true"){
            mMouseGen.doMouseMove(details.target, details.point, details.button);
        }
    }
}

/*!
 Set point from parameters.
 */
void MouseHandler::setPoint(TasCommand& command, TapDetails& details)
{
    if(command.parameter("useCoordinates") == "true"){
        int x = command.parameter("x").toInt();
        int y = command.parameter("y").toInt();
        details.point.setX(x);
        details.point.setY(y);
        details.extraIdentifier = QString::number(details.point.x()) +"_"+ QString::number(details.point.y());
    }
}

Tapper::Tapper(MouseHandler* handler, MouseHandler::TapDetails details, int count, int interval)
{
    mHandler = handler;
    mMaxCount = count;
    mTapCount = 0;
    mDetails = details;
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(tap()));
    mTimer.start(interval);
}

void Tapper::tap()
{
    mTapCount++;

    mHandler->press(mDetails);
    mHandler->release(mDetails);

    if( mTapCount >= mMaxCount ){
        mTimer.stop();
        deleteLater();
    }
}
