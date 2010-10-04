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

#ifndef TASMULTIGESTURERUNNER_H
#define TASMULTIGESTURERUNNER_H

#include <QList>
#include <QHash>

#include "tasgesture.h"
#include "tastoucheventgenerator.h"

class TasMultiGestureRunner : public QObject
{
    Q_OBJECT
public:  
    TasMultiGestureRunner(QList<TasGesture*> gestures, QObject* parent=0);
    ~TasMultiGestureRunner();

protected slots:
    void timerEvent(qreal);
	void finished();

private:
	void startGesture();

private:
	QTimeLine mTimeLine;
	QHash<TasGesture*,qreal> mGestures;
	TasTouchEventGenerator mTouchGen;	
	QHash<TasGesture*, QList<TasTouchPoints> > mPreviousPoints;
};

#endif
