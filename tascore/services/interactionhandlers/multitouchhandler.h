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
#include "tasgesture.h"

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
	TasGesture* mGesture;
};

#endif
