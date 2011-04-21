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

#ifndef TASGESTURE_H
#define TASGESTURE_H

#include <QList>
#include <QPoint>
#include <QTimeLine>
#include <QLine>

#include "tastoucheventgenerator.h"
#include "tasconstants.h"
#include "mousehandler.h"

const int FRAME_RANGE_DIV = 10;

class TasGesture
{
public:
    TasGesture(TargetData data);
    virtual ~TasGesture(){}

	virtual QList<TasTouchPoints> startPoints() = 0;
	virtual QList<TasTouchPoints> pointsAt(qreal value) = 0;
	virtual QList<TasTouchPoints> endPoints() = 0;
	virtual bool isMultiTouch() = 0;
	MouseHandler::PointerType pointerType();
	void setPointerType(MouseHandler::PointerType type);
	QWidget* getTarget();
	QGraphicsItem* getTargetItem();	
	Qt::MouseButton getMouseButton();
	int getDuration(){return mDuration;}
	bool isDrag(){return mIsDrag;}
	bool isPress(){return mPress;}
	bool isRelease(){return mRelease;}
	QString touchPointIdKey(){return mTouchPointIdKey;}
	void setUseTapScreen(bool useTapScreen) { mUseTapScreen=useTapScreen; }
	bool getUseTapScreen() { return mUseTapScreen; }

protected:
	TasTouchPoints makeTouchPoint(QPoint pos, QPoint lastPos=QPoint(), QPoint startPos=QPoint());
	QList<TasTouchPoints> listFromPoint(QPoint pos, QPoint lastPos=QPoint(), QPoint startPos=QPoint());

protected:
	bool mIsDrag;
	bool mPress;
	bool mRelease;
	bool mUseTapScreen;
	int mDuration;
	QWidget* mTarget;
	QGraphicsItem* mTargetItem;
	QString mTouchPointIdKey;
	QPoint mStartPoint;
	QPoint mLastPoint; 
	//for multitouch
	QList<QPoint> mStartPoints;
	QList<QPoint> mLastPoints;
	Qt::MouseButton mButton;
	MouseHandler::PointerType mPointerType;
};


class LineTasGesture : public TasGesture
{
public:
    LineTasGesture(TargetData data, QLineF gestureLine);
	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointsAt(qreal value);
	QList<TasTouchPoints> endPoints();  
	bool isMultiTouch(){return false;}

private:
	QLineF mGestureLine;
};

class PointsTasGesture : public TasGesture
{
public:
    PointsTasGesture(TargetData data, QList<QPoint> points);  

	void setIntervals(QList<int> intervals);
	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointsAt(qreal value);
	QList<TasTouchPoints> endPoints();  

	bool isMultiTouch(){return false;}  

protected:
	void calculateAnimation();
	int getDuration();

private:
	QList<QPoint> mPoints;
	QList<int> mIntervals;
	bool mUseIntervals;
};

class PinchZoomTasGesture : public TasGesture
{
public:
    PinchZoomTasGesture(TargetData data, QLineF line1, QLineF line2);
    ~PinchZoomTasGesture();
  
	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointsAt(qreal value);
	QList<TasTouchPoints> endPoints();
	bool isMultiTouch(){return true;}

private:
	QLineF mGestureLine1;
	QLineF mGestureLine2;
};

class SectorTasGesture : public TasGesture
{
public:
    SectorTasGesture(TargetData data, QLineF line, int distance);

	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointsAt(qreal value);
	QList<TasTouchPoints> endPoints();
	bool isMultiTouch(){return true;}

private:
	QList<QPoint> activePoints();

private:
	QLineF mGestureLine;
	qreal mStartAngle;
	int mDistance;
};

class ArcsTasGesture : public TasGesture
{
public:
    ArcsTasGesture(TargetData data, QLineF line1, QLineF line2, int distance);

	QList<TasTouchPoints> startPoints();
	QList<TasTouchPoints> pointsAt(qreal value);
	QList<TasTouchPoints> endPoints();
	bool isMultiTouch(){return true;}

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
