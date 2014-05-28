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
 


#ifndef RECORDERSERVICE_H
#define RECORDERSERVICE_H

#include <QEvent>

#include "tasservicebase.h"
#include "tasqtdatamodel.h"
#include "tastraverseinterface.h"

class RecorderService : public QObject, public TasServiceBase 
{
public:
    RecorderService(QObject* parent = 0);
	~RecorderService();

    bool eventFilter(QObject *target, QEvent *event);        

	/*!
	  From ServiceInterface
	*/
	bool executeService(TasCommandModel& model, TasResponse& response);
	QString serviceName()const { return RECORD_EVENTS; }

private:	
	void performRecorderCommands(TasCommandModel& model, TasResponse& response);
	void start();
	void printTargetDetails(QObject* target, TasObject& targetObj);
private:
	TasDataModel* mTasModel;
	TasObject* mTasEvents; 
	int eventCounter;
	QHash<QString, TasTraverseInterface*> mTraversers;
};

#endif
