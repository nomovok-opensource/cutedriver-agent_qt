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
 


#ifndef MOUSEHANDLER_H
#define MOUSEHANDLER_H

#include <QApplication>
#include <QPoint>
#include <QMouseEvent>
#include <QCursor>
#include <QWidget>
#include <QTimer>

#include "uicommandservice.h"

class Tapper;

struct TasTouchPoints
{
    QPoint screenPoint;
    QPoint lastScreenPoint;
    QPoint startScreenPoint;
};


class MouseHandler : public InteractionHandler
{
public:
    MouseHandler();
    ~MouseHandler();
  	
	
    virtual bool executeInteraction(TargetData data);
  
protected:
    void doMouseDblClick(QWidget* target, Qt::MouseButton button, QPoint point);
    void doMousePress(QWidget* target, QGraphicsItem* targetItem, Qt::MouseButton button, QPoint point); 
    void doMouseRelease(QWidget* target, QGraphicsItem* targetItem, Qt::MouseButton button, QPoint point);
    void doMouseMove(QWidget* target, QGraphicsItem* targetItem, QPoint point, Qt::MouseButton button=Qt::NoButton);    
    void moveCursor(QPoint point);    
    void checkMoveMouse(TasCommand& command);
    void setPoint(QPoint& point, TasCommand& command);
    void checkMoveMouse(TasCommand& command, QPoint point);
    Qt::MouseButton getMouseButton(TasCommand& command);
    void doScroll(QWidget* target, QPoint& point, int delta, Qt::MouseButton button,  Qt::Orientation orient);
    void sendMouseEvent(QWidget* target, QMouseEvent* event);

    void doTouchBegin(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points);
    void doTouchUpdate(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points);
    void doTouchEnd(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points);
    QList<QTouchEvent::TouchPoint> convertToTouchPoints(QWidget* target, QGraphicsItem* targetItem, Qt::TouchPointState state,
                                                        QList<TasTouchPoints> points);
    QTouchEvent::TouchPoint makeTouchPoint(QWidget* target, QGraphicsItem* targetItem, TasTouchPoints points,
                                           Qt::TouchPointState state, int id, bool primary = true);
    void sendTouchEvent(QWidget* target, QTouchEvent* event);

    bool acceptsTouchEvent(QWidget* target, QGraphicsItem* targetItem);

    QList<TasTouchPoints> toTouchPoints(QPoint point);

private:
    int mTouchPointCounter;
    QHash<QString,QList<int>* > mTouchIds;
    bool mUseTapScreen; // If true, operation system native functionality will be used in mouse movements
    friend class Tapper;
};

class Tapper : public QObject
{
    Q_OBJECT

    public:
    Tapper(MouseHandler* handler, int count, int interval, QWidget* target, QGraphicsItem* 
           targetItem, Qt::MouseButton button, QPoint point);
	
private slots:
    void tap();

	
private:
    QTimer mTimer;
    QWidget* mTarget; 
    QGraphicsItem* mTargetItem;
    Qt::MouseButton mButton;
    QPoint mPoint;
    MouseHandler* mHandler;
    int mMaxCount;
    int mTapCount;
};

#endif
