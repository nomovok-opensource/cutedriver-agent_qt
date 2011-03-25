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

#ifndef TASGESTURERUNNER_H
#define TASGESTURERUNNER_H

#include "tasgesture.h"
#include "tastoucheventgenerator.h"
#include "tasmouseeventgenerator.h"

class TasGestureRunner : public QObject
{
    Q_OBJECT
public:  
    TasGestureRunner(TasGesture* gesture, QObject* parent=0);
    ~TasGestureRunner();

    bool eventFilter(QObject *target, QEvent *event);

protected slots:
    void timerEvent(qreal);
	void finished();
	void releaseMouse();

private:
	void startGesture();
	void move(QList<TasTouchPoints> points, bool force=false);
	bool noMovement(QList<TasTouchPoints> points);

private:
	QTimeLine mTimeLine;
	TasGesture* mGesture;
	TasMouseEventGenerator mMouseGen;
	TasTouchEventGenerator mTouchGen;	
	QList<TasTouchPoints> mPreviousPoints;
};





#endif
