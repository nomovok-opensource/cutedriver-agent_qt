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
 
 


#ifndef TASQTMEMLOGSERVICE_H
#define TASQTMEMLOGSERVICE_H


#include <QObject>
#include <QTimer>

#include <tasconstants.h>
#include <tasservicemanager.h>

#if defined(TAS_NOLOCALSOCKET)
#include <QTcpSocket>
#else
#include <QLocalSocket>
#endif

class TasQtMemLogService : public QObject, public ResponseHandler
{
    Q_OBJECT

public:
    TasQtMemLogService(QObject *parent = 0);
    ~TasQtMemLogService();
  
public slots:
    void unReqisterServicePlugin();
	void serviceResponse(TasMessage& response);
    
private slots:
	void sendRegisterMessage();
    void connectionClosed();
	void timeout();

private:
	QString makeReqisterMessage(QString command, QMap<QString,QString> attributes);
	void initializeConnections();
	void cleanConnections();

private:
#if defined(TAS_NOLOCALSOCKET)
    QTcpSocket* mServerConnection;
#else
    QLocalSocket* mServerConnection;
#endif
 	TasServiceManager* mServiceManager;
    TasClientSocket* mSocket;
    bool mRegistered;   
    bool mConnected;
	QTimer mRegisterTime;
	qint32 mMessageId;
	bool mDoNotReconnect;
};

#endif


