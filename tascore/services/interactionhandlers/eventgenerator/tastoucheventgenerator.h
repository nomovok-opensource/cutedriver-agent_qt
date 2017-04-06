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

#ifndef TASTOUCHEVENTGENERATOR_H
#define TASTOUCHEVENTGENERATOR_H

#include <QApplication>
#include <QWidget>
#include <QGraphicsItem>
#include <QPoint>
#include <QTouchEvent>
#include <QHash>
#include <QTouchDevice>

#include "uicommandservice.h"
#include "taseventtarget.h"

struct TasTouchPoints
{
    QPoint screenPoint;
    QPoint lastScreenPoint;
    QPoint startScreenPoint;
};

class TasTouchEventGenerator : public QObject
{
    Q_OBJECT
public:
    TasTouchEventGenerator(QObject* parent=0);
    ~TasTouchEventGenerator();

    void doTouchBegin(const TasEventTarget& target, QPoint point, QString identifier);
    void doTouchUpdate(const TasEventTarget& target, QPoint point, QString identifier);
    void doTouchEnd(const TasEventTarget& target, QPoint point, QString identifier);

	QList<QTouchEvent::TouchPoint> convertToTouchPoints(TargetData targetData, Qt::TouchPointState state);

    QList<QTouchEvent::TouchPoint> convertToTouchPoints(const TasEventTarget& target, Qt::TouchPointState state,
                                                        QList<TasTouchPoints> points, QString identifier=QString());
    QTouchEvent::TouchPoint makeTouchPoint(const TasEventTarget& target, TasTouchPoints points, Qt::TouchPointState state, int id);

    QList<TasTouchPoints> toTouchPoints(QPoint point);
    TasTouchPoints toTouchPoint(QPoint point);
    void sendTouchEvent(const TasEventTarget& target, QTouchEvent* event);
	bool areIdentical(QList<TasTouchPoints> points1, QList<TasTouchPoints> points2);

public slots:
    void doTouchBegin(const TasEventTarget& target, QList<TasTouchPoints> points, QString identifier=QString());
    void doTouchUpdate(const TasEventTarget& target, QList<TasTouchPoints> points, QString identifier=QString());
    void doTouchEnd(const TasEventTarget& target, QList<TasTouchPoints> points, QString identifier=QString());

private:
    void createTouchDevice();

private:
    static int mTouchPointCounter;
    QTouchDevice* mTouchDevice;
};

#endif
