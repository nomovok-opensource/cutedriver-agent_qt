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
 


#ifndef MOUSEHANDLER_H
#define MOUSEHANDLER_H

#include <QApplication>
#include <QPoint>
#include <QMouseEvent>
#include <QCursor>
#include <QWidget>
#include <QTimer>
#include <QAction>

#include "uicommandservice.h"
#include "tasmouseeventgenerator.h"          
#include "tastoucheventgenerator.h"

class Tapper;

const char* const POINTER_TYPE = "eventType";


class MouseHandler : public InteractionHandler
{
public:
    enum PointerType{
 	    TypeMouse = 0,
		TypeTouch = 1,
		TypeBoth = 2
	};

	struct TapDetails{
        QWidget* target;
	    Qt::MouseButton button;
	    QPoint point;
   	    //for touch events to generate touch point ids
	    QString identifier;
 	    MouseHandler::PointerType pointerType;
 	    TasCommand* command;
	};

public:
     MouseHandler();
    ~MouseHandler();
  	
	
    virtual bool executeInteraction(TargetData data);
  
	static Qt::MouseButton getMouseButton(TasCommand& command);

private:
	void performActionEvent(TapDetails details);
    void checkMoveMouse(TasCommand& command);
    void setPoint(TasCommand& command, TapDetails& details);
    void checkMoveMouse(TasCommand& command, QPoint point);   
	void press(TapDetails details);
	void move(TapDetails details);
	void release(TapDetails details);

	MouseHandler::TapDetails makeDetails(TargetData data);
	QAction* getAction(QWidget* widget, int id);

private:
	TasMouseEventGenerator mMouseGen;
	TasTouchEventGenerator mTouchGen;
	QStringList mCommands;
	friend class Tapper;
};

class Tapper : public QObject
{
    Q_OBJECT
public:
    Tapper(MouseHandler* handler, MouseHandler::TapDetails details, int count, int interval);
	
private slots:
    void tap();
	
private:
	MouseHandler* mHandler;
    QTimer mTimer;
	MouseHandler::TapDetails mDetails;
    int mMaxCount;
    int mTapCount;
};

#endif
