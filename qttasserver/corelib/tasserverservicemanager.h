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
 

#ifndef TASSERVERSERVICEMANAGER_H
#define TASSERVERSERVICEMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QTimer>
#include <tasservicemanager.h>
#include <tasqtcommandmodel.h>
#include <tassocket.h>

#include "tasclientmanager.h"
#include "tasservercommand.h"

class ResponseWaiter;

class TasServerServiceManager : public QObject, public TasServiceManager, public ResponseHandler
{
    Q_OBJECT

public:
    TasServerServiceManager(QObject* parent=0);
    ~TasServerServiceManager();

	void serviceResponse(TasMessage& response);

protected:
	void handleServiceRequest(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId);
	QString serviceErrorMessage(){return "QtTasserver does not support the given service: ";}

private slots:
	void removeWaiter(qint32 responseId);

private:	
	QHash<qint32, ResponseWaiter*> reponseQueue;
	TasClientManager* mClientManager;
};

class ResponseWaiter : public QObject
{
    Q_OBJECT

public:
    ResponseWaiter(qint32 responseId, TasSocket* relayTarget, int timeout=10000);
    ~ResponseWaiter();

	void setResponseFilter(ResponseFilter* filter);

	void sendResponse(TasMessage& response);

signals:
	void responded(qint32 responseId);

private slots:	
	void timeout();
	void socketClosed();

private:
	qint32 mResponseId;
	QTimer mWaiter;
	TasSocket *mSocket;
	ResponseFilter* mFilter;
};

#endif
