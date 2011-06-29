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

#ifndef CUCUMBERWIREPROTOCOLSERVER_H
#define CUCUMBERWIREPROTOCOLSERVER_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QVariantList>
#include <QWeakPointer>

class QTimer;
class QTcpServer;
class QIODevice;
class CucumberWireprotocolServer_StepData;
class CucumberApplicationManager;

class CucumberWireprotocolServer : public QObject {
    Q_OBJECT

public:

    CucumberWireprotocolServer(quint16 port, QObject *parent = 0);
    ~CucumberWireprotocolServer();
    QString addressString();
    QString lastErrorString() { return mLastErrorString; }


    // invokable methods meant to be called from step defintion methods
    Q_INVOKABLE void cucumberFailResponse(const QString &errorString);
    Q_INVOKABLE void cucumberSuccessResponse();
    Q_INVOKABLE void cucumberResponse(const QVariantList &outMsg);

    // step defintion method that just dumps the data, for testing/reference
    Q_INVOKABLE void invokeDebugDump(const QString &regExpPattern, const QVariantList &args, QObject *sender);

public slots:
    void registerStep(const QRegExp &regExp, QObject *object, const char *method, const QString &sourceFileSpec);
    void closeAllClients();
    void close();
    int start();

signals:
    void gotJSONMessage(QVariant data, QIODevice *connection);

private slots:
    void reRegisterSteps();
    void handleNewConnect();
    void connectionDisconnect();
    void connectionError();
    void connectionReadyRead();
    void connectionBytesWritten(qint64 bytes);

    void handleJSONMessage(QVariant data, QIODevice *connection);
    QVariantList findMatchingSteps(const QString &name);
    bool startStep(const QVariantMap &args, QIODevice *connection);
    void responseTimeout();
    void setResponseTimeout(int sec, QIODevice *connection);
    void resetResponseTimeout();

    void writeResponse(const QVariantList &outMsg, QIODevice *connection);


private:
    QList<CucumberWireprotocolServer_StepData*> mSteps;
    QMap<QIODevice*, QByteArray> mReadBufferMap;
    QString mLastErrorString;
    QTcpServer *mServer;
    quint16 mListenPort;

    bool mPendingResponse;
    QTimer *mResponseTimeoutTimer;
    QWeakPointer<QIODevice> mPendingConnection;
    CucumberApplicationManager *mAppManager;
};

#endif // CUCUMBERWIREPROTOCOLSERVER_H
