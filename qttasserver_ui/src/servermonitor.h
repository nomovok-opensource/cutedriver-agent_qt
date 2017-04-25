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



#ifndef SERVERMONITOR_H
#define SERVERMONITOR_H

#include <QObject>
#include <QProcess>
#include <QTimer>

#include "tassocket.h"
#include "tasconstants.h"

#include <QTcpSocket>
#include <QLocalSocket>


class TasClient;

enum MonitorState
{
  STATUS = 0,
  STOP = 1,
  START = 2,
  RESTART = 3,
  TEST_CONNECTION = 4
};


class ServerMonitor : public QObject
{
    Q_OBJECT

public:

    ServerMonitor(QObject* parent=0);
    ~ServerMonitor();

signals:
    void serverDebug(const QString& text);
    void serverState(const QString& state);
    void beginMonitor();
    void stopMonitor();
    void enableReBinding(const QString& currentBinding);
    void disableReBinding();

public slots:
    void serverState();
    void restartServer();
    void stopServer();
    void startServer();
    void loadPlugins();
    void setAnyBinding();
    void setLocalBinding();

private slots:
    void error(const QString& message);
    void info(const QString& message);
    void serverResponse(const QString& message);

private:
    void killServer();

private:
    TasClient* mClient;
    QTimer mTimer;
    MonitorState mState;
    qint32 mMessageCount;
};

class TasClient : public QObject, public ResponseHandler
{
    Q_OBJECT
public:

    TasClient();
    ~TasClient();

    void sendMessage(const QString& message);
    void serviceResponse(TasMessage& response);

signals:
    void info(const QString& message);
    void error(const QString& message);
    void serverResponse(const QString& message);

protected slots:
    void connectionTimeout();
    void sendData(const QString& message);
    void connectionClosed();

private:
    bool connectToServer();

private:
    bool mConnected;
    QTimer mTimer;
    TasSocket *mSocket;
#if defined(TAS_NOLOCALSOCKET)
    QTcpSocket* mServerConnection;
#else
    QLocalSocket* mServerConnection;
#endif
    int mMessageId;
    bool mSending;
};


#endif
