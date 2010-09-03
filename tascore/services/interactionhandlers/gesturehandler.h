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
#include <QTimeLine>
#include <QLineF>
#include <QList>

#include "mousehandler.h"

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
	QWidget* mWidget;
    QGraphicsItem* mItem;

private:
	GesturePath* mGesturePath;
	QTimeLine* mTimeLine;
	QPoint mPrevious;
};

class GesturePath
{
public:
  virtual ~GesturePath(){}

  virtual QPoint startPoint() = 0;
  virtual QPoint pointAt(qreal value) = 0;
  virtual QPoint endPoint() = 0;
  virtual void setIntervals(QList<int> intervals) = 0;
  virtual bool useIntervals() =0 ;
};

class LinePath : public GesturePath
{
public:
  LinePath(QLineF gestureLine);
  QPoint startPoint();
  QPoint pointAt(qreal value);
  QPoint endPoint();  
  void setIntervals(QList<int> intervals){ Q_UNUSED(intervals); return; }
  bool useIntervals(){return false;}

private:
  QLineF mGestureLine;
  
};

class PointsPath : public GesturePath
{
public:
  PointsPath(QList<QPoint> points);  
  void setIntervals(QList<int> intervals);

  QPoint startPoint();
  QPoint pointAt(qreal value);
  QPoint endPoint();
  bool useIntervals();
  
private:
  void calculateAnimation();
  int getDuration();

private:
  QList<QPoint> mPoints;
  QList<int> mIntervals;
  bool mUseIntervals;
};
#endif
