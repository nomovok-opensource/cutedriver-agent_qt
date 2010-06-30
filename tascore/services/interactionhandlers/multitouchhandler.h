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
 


#ifndef MULTITOUCHHANDLER_H
#define MULTITOUCHHANDLER_H

#include <QApplication>
#include <QObject>
#include <QTimeLine>
#include <QLineF>
#include <QList>

#include "gesturehandler.h"

class MultitouchGesturePath;

class MultitouchHandler : public GestureHandler
{
  Q_OBJECT

public:
    MultitouchHandler(QObject* parent=0);
    ~MultitouchHandler();
  
	bool executeInteraction(TargetData data);
	bool executeMultitouchInteraction(QList<TargetData> dataList);

protected:
	void beginGesture();

protected slots:
    void timerEvent(qreal);
	void finished();

private:
	bool validateZoomParams(TasCommand& command);
	bool validateRotationParams(TasCommand& command);
	bool makePinchZoomGesture(TasCommand& command, QPoint point);
	bool makeRotationGesture(TasCommand& command, QPoint point);
	QLineF makeLine(QPoint start, int length, int angle);

private:
	MultitouchGesturePath* mGesturePath;
};

class MultitouchGesturePath
{
public:
    virtual ~MultitouchGesturePath(){};

    virtual QList<TasTouchPoints> startPoints() = 0;
    virtual QList<TasTouchPoints> pointAts(qreal value) = 0;
    virtual QList<TasTouchPoints> endPoints() = 0;

protected:
	TasTouchPoints makeTouchPoint(QPoint pos, QPoint lastPos=QPoint(), QPoint startPos=QPoint());
	QList<QPoint> mStartPoints;
	QList<QPoint> mLastPoints;
};

class PinchZoomGesturePath : public MultitouchGesturePath
{
public:
    PinchZoomGesturePath(QLineF line1, QLineF line2);
    ~PinchZoomGesturePath();
  
	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointAts(qreal value);
	QList<TasTouchPoints> endPoints();

private:
	QLineF mGestureLine1;
	QLineF mGestureLine2;
};

class SectorGesturePath : public MultitouchGesturePath
{
public:
    SectorGesturePath(QLineF line, int distance);

	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointAts(qreal value);
	QList<TasTouchPoints> endPoints();

private:
	QList<QPoint> activePoints();

private:
	QLineF mGestureLine;
	qreal mStartAngle;
	int mDistance;
};

class ArcsGesturePath : public MultitouchGesturePath
{
public:
    ArcsGesturePath(QLineF line1, QLineF line2, int distance);

	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointAts(qreal value);
	QList<TasTouchPoints> endPoints();

private:
	QList<QPoint> activePoints();

private:
	QLineF mGestureLine1;
	QLineF mGestureLine2;
	qreal mStartAngle1;
	qreal mStartAngle2;
	int mDistance;
};

#endif
