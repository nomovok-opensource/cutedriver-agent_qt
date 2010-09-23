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
 


#ifndef TASMOUSEEVENTGENERATOR_H
#define TASMOUSEEVENTGENERATOR_H

#include <QApplication>
#include <QPoint>
#include <QWidget>
#include <QMouseEvent>
#include <QCursor>

class TasMouseEventGenerator : public QObject
{
   Q_OBJECT
public:
    TasMouseEventGenerator(QObject* parent=0);
    ~TasMouseEventGenerator();

	void setUseTapScreen(bool use);
  
public slots:	
	void doMousePress(QWidget* target, Qt::MouseButton button, QPoint point); 
    void doMouseRelease(QWidget* target, Qt::MouseButton button, QPoint point);
    void doMouseMove(QWidget* target, QPoint point, Qt::MouseButton button=Qt::NoButton);    
    void doScroll(QWidget* target, QPoint& point, int delta, Qt::MouseButton button,  Qt::Orientation orient);
    void doMouseDblClick(QWidget* target, Qt::MouseButton button, QPoint point);
	void moveCursor(QPoint point);

private:
	void sendMouseEvent(QWidget* target, QMouseEvent* event);


private:
	bool mUseTapScreen;
};

#endif
