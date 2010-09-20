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
 


#ifndef EVENTSERVICE_H
#define EVENTSERVICE_H

#include <QEvent>
#include <QStringList>
#include <QDateTime>
#include <QHash>

#include "tasservicebase.h"
#include "tasqtdatamodel.h"

class TasEventFilter;

class EventService : public TasServiceBase 
{
public:
    EventService();
	~EventService();

	/*!
	  From ServiceInterface
	*/
	bool executeService(TasCommandModel& model, TasResponse& response);
	QString serviceName()const { return COLLECT_EVENTS; }

	void enableEvents(QString targetId, QObject* target, QStringList eventsToListen);
	void addProcessStartEvent(QDateTime startTime);

private:	
	void performEventCommands(TasCommandModel& model, TasResponse& response);
	TasEventFilter* getFilterForTarget(TasTarget* commandTarget, bool create=false);

private:
	QHash<QString, TasEventFilter*> mEventFilters;
};

class TasEventFilter : public QObject
{
public:
    TasEventFilter(QObject* target, QObject* parent = 0);
    ~TasEventFilter();

	void startFiltering(QStringList eventsToListen);

    bool eventFilter(QObject *target, QEvent *event);        
	QByteArray* getEvents();

	void addStartTime(QDateTime startTime);

private:
	void addMouseEventDetails(QEvent *event, TasObject& eventObject);

private:	
	QObject* mTarget;
	TasDataModel* mTasModel;
	TasObject* mTasEvents; 
    QStringList mEventsToListen;
};

#endif
