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
 



#ifndef TESTABILITYSERVICE_H
#define TESTABILITYSERVICE_H

//#define USE_TAS_OS_SPEC

#include <QObject>
#include <QTimer>
#include <QMap>

#include "tasconstants.h"

#if defined(TAS_NOLOCALSOCKET)
#include <QTcpSocket>
#else
#include <QLocalSocket>
#endif

#include "tassocket.h"
#include "tasqtcommandmodel.h"
#include "tasservicemanager.h"

class EventService;
class FixtureService;

class TestabilityService : public QObject, public ResponseHandler
{
    Q_OBJECT

public:
    TestabilityService(QObject *parent = 0);
    ~TestabilityService();

public:
	bool eventFilter(QObject *target, QEvent *event);

signals:
	void registered();
	void unRegistered();
  
public slots:
	void registerPlugin();
    void unReqisterServicePlugin();
	void serviceResponse(TasMessage& response);
    
private slots:
	void sendRegisterMessage();
    void connectionClosed();
	void timeout();

private:
    void initializeServiceManager();
	QString makeReqisterMessage(QString command, QMap<QString,QString> attributes);
	void loadStartUpParams(QString appName);
	void enableSignalTracking(QString signal, QString timeStamp);
	void prepareForDeletion();
	void initializeConnections();

private:
   
 	TasServiceManager* mServiceManager;

#if defined(TAS_NOLOCALSOCKET)
    QTcpSocket* mServerConnection;
#else
    QLocalSocket* mServerConnection;
#endif

    TasClientSocket* mSocket;
    QString mPluginId;    
    bool mRegistered;   
    bool mConnected;
    EventService* mEventService;
    FixtureService* mFixtureService;
	QTimer mRegisterTime;
	QTimer mRegisterWatchDog;
	QTimer mPaintTracker;
	qint32 mMessageId;
	int mPaintEventCounter;
};


class TestabilityLoader : public QObject
{
  Q_OBJECT

public:
   TestabilityLoader();

public slots:
   void load();
   void unload();

private:
   TestabilityService *mService;
};

#endif

