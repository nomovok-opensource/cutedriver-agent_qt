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

#ifndef TASGESTURECOGNIZERS_H
#define TASGESTURECOGNIZERS_H

#include <QLineF>
#include <QStringList>

#include "uicommandservice.h"
#include "tasgesture.h"

class TasGestureUtils : public TestabilityUtils
{
public:  
    TasGestureUtils(){};
    ~TasGestureUtils(){};
  	void doTransform(QGraphicsItem* targetItem, QLineF& gestureLine);
	QPoint getPoint(TasCommand& command);
	QPoint getTargetPoint(TasCommand& command);
	int getDistance(TasCommand& command);
	int getDirection(TasCommand& command);
	QLineF makeLine(QPoint start, int length, int angle);
};

class TasGestureRecognizer
{
public:
    virtual ~TasGestureRecognizer(){}
    virtual TasGesture* create(TargetData data) = 0;
	virtual bool isSupportedType(const QString& gestureType) = 0;
};

class LineTasGestureRecognizer : public TasGestureRecognizer
{
public:
    LineTasGestureRecognizer();
    TasGesture* create(TargetData data);
	bool isSupportedType(const QString& gestureType);

private:
	QStringList mTypes;
	TasGestureUtils mUtils;
};

class PointsTasGestureRecognizer : public TasGestureRecognizer
{
public:
    PointsTasGestureRecognizer();
    TasGesture* create(TargetData data);
	bool isSupportedType(const QString& gestureType);
};

class PinchZoomTasGestureRecognizer : public TasGestureRecognizer
{
public:
    PinchZoomTasGestureRecognizer();
    TasGesture* create(TargetData data);
	bool isSupportedType(const QString& gestureType);

private:
	bool validateZoomParams(TasCommand& command);

private:
	TasGestureUtils mUtils;
};

class RotationTasGestureRecognizer : public TasGestureRecognizer
{
public:
    RotationTasGestureRecognizer();
    TasGesture* create(TargetData data);
	bool isSupportedType(const QString& gestureType);
private:
	bool validateRotationParams(TasCommand& command);

private:
	TasGestureUtils mUtils;
};


#endif
