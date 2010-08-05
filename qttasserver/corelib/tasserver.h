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
 


#ifndef TASSERVER_H
#define TASSERVER_H

#include <QObject>
#include <QProcess>
#include <QHash>
#include <QList>
#include <QTcpServer>
#include <QThread>

#include <tassocket.h>
#include <tasqtcommandmodel.h>
#include <tasconstants.h>
#include "tasserverservicemanager.h"

#include "tasclientmanager.h"

#if defined(TAS_USELOCALSOCKET)
#include <QLocalServer>
#endif

class TasTcpServer;
class TasLocalServer;

class TasServer : public QObject
{
    Q_OBJECT

public:           
    TasServer(QObject *parent = 0 );
    ~TasServer();
     
    bool startServer();        
    const QString& getErrorMessage(); 

    int getServerPort();
    QString getServerAddress();
	const QString& getServerVersion();
    
public slots:
    void closeServer();
    void killAllStartedProcesses();

private:
	void createServers();

private:                 

	TasTcpServer* mTcpServer;
#if defined(TAS_USELOCALSOCKET)
	TasLocalServer* mLocalServer;
#else
	TasTcpServer* mInternalTcpServer;
#endif

	TasClientManager* mClientManager;
	TasServerServiceManager* mServiceManager;
	QString errorMessage;
};


class TasTcpServer : public QTcpServer
{
    Q_OBJECT

public:
    TasTcpServer(int port, TasServerServiceManager& serviceManager, QObject * parent = 0);
	~TasTcpServer();

	bool start();

public slots:
	void restartServer();
	void socketError(QAbstractSocket::SocketError socketError);
	
protected:
	void incomingConnection ( int socketDescriptor );

private:
	int mPort;
	int mConnectionCount;
	TasServerServiceManager& mServiceManager;
};

#if defined(TAS_USELOCALSOCKET)
class TasLocalServer : public QLocalServer
{
    Q_OBJECT

public:
    TasLocalServer(const QString& name, TasServerServiceManager& serviceManager, QObject * parent = 0);
	~TasLocalServer();

	bool start();

protected:

	void incomingConnection ( quintptr socketDescriptor );

private:
	QString mName;
	int mConnectionCount;
	TasServerServiceManager& mServiceManager;
};
#endif

#endif
