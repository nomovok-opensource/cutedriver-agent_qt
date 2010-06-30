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
/*
 * resourceloggingservice.h
 */

#ifndef RESOURCELOGGINGSERVICE_H_
#define RESOURCELOGGINGSERVICE_H_

#include "tasservercommand.h"
#include <tasconstants.h>
#include <QTimer>

class ResourceLoggingService : public TasServerCommand
{
public:
    ResourceLoggingService();
    ~ResourceLoggingService();
    bool executeService(TasCommandModel& model, TasResponse& response);
    QString serviceName() const { return RESOURCE_LOGGING_SERVICE; }
    
};

class MemLogServerWaiter : public QObject
{
    Q_OBJECT
public:
    MemLogServerWaiter(TasSocket* requester, TasClient *target, const QString& commandXml, qint32 messageId);
    ~MemLogServerWaiter();

private slots:
	void clientRegistered(const QString& processId);
    void timeout();
	void socketClosed();
	void crashed();

private:
	QTimer mWaiter;
	QString mCommandXml;
	qint32 mMessageId;
	TasSocket* mSocket;   
	TasClient* mTarget;
};

#endif
