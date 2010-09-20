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
 


#ifndef GESTUREHANDLER_H
#define GESTUREHANDLER_H

#include <QApplication>
#include <QObject>
#include <QLineF>
#include <QList>

#include "mousehandler.h"
#include "tasgesture.h"

class GesturePath;

struct TargetDetails
{
  int distance;    
  int direction;
  int duration;
  QString targetId;
  QString targetType;
  bool press;
  bool release;
  bool isDrag;
  Qt::MouseButton button;    
  QWidget* widget;
  QGraphicsItem* item;
};

class GestureHandler : public QObject, public MouseHandler
{
  Q_OBJECT

public:
    GestureHandler(QObject* parent=0);
    ~GestureHandler();
  
	bool executeInteraction(TargetData data);
    bool eventFilter(QObject *target, QEvent *event);        

protected:
	virtual void beginGesture();
	void startGesture();
	TargetDetails getParameters(TasCommand& command);

	QLineF makeGestureLine(TargetData data);

protected slots:
    virtual void timerEvent(qreal);
	virtual void finished();
	void releaseMouse();

private:
    void setNewPoint();
	QPoint getTargetPoint(TargetDetails targetDetails);

protected:
	TargetDetails mTargetDetails;

private:
	TasGesture* mGesture;
	QTimeLine* mTimeLine;
	QPoint mPrevious;
};

#endif
