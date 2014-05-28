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

#include "tasservicemanager.h"
#include "tasqtcommandmodel.h"
#include "tassocket.h"
#include "taspluginloader.h"
#include "tasclientmanager.h"
#include "tasservercommand.h"
#include "tasservercommand.h"
#include "tasextensioninterface.h"

class ResponseWaiter;

class TasServerServiceManager : public QObject, public TasServiceManager, public ResponseHandler
{
    Q_OBJECT

public:
    TasServerServiceManager(QObject* parent=0);
    ~TasServerServiceManager();

	void serviceResponse(TasMessage& response);

	QByteArray& responseHeader() const;

protected:
	void handleServiceRequest(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId);
	QString serviceErrorMessage(){return "QtTasserver does not support the given service: ";}

private slots:
	void removeWaiter(qint32 responseId);

private:
	void loadExtensions();
	void loadExtension(const QString& filePath);
    bool extensionHandled(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId);
#ifdef Q_OS_SYMBIAN   
    bool appendVkbData(TasCommandModel& commandModel, QByteArray& data);
#endif
    void handleClientLess(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId);
    void getNativeUiState(qint32 responseId, TasCommandModel& commandModel);
private:	
	QHash<qint32, ResponseWaiter*> mResponseQueue;
	TasClientManager* mClientManager;
	QList<TasExtensionInterface*> mExtensions;
    TasPluginLoader mPluginLoader;    
};

class ResponseWaiter : public QObject
{
    Q_OBJECT

public:
    ResponseWaiter(qint32 responseId, TasSocket* relayTarget, int timeout=10000);
    ~ResponseWaiter();

	void setResponseFilter(ResponseFilter* filter);
	void sendResponse(TasMessage& response);

	void appendPlatformData(const QByteArray& data);
    void okToRespond();

signals:
	void responded(qint32 responseId);

private slots:	
	void timeout();
	void socketClosed();

private:
	void cleanup();
    void sendMessage();

private:
	qint32 mResponseId;
	QTimer mWaiter;
    QPointer<TasSocket> mSocket;
	ResponseFilter* mFilter;
	QByteArray mPlatformData;
    bool mCanRespond;
    bool mPluginResponded;
    TasMessage mMessageToSend;
};

class CloseFilter : public ResponseFilter
{
public:
    CloseFilter(TasCommandModel& model);
    ~CloseFilter();
	void filterResponse(TasMessage& response);

private:
	quint64 mPid;
    int mWaitTime;
    bool mKill;
	bool mPassThrough;
};



#endif
