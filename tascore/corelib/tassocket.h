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
 


#ifndef TASSOCKET_H
#define TASSOCKET_H

#include <QObject>
#include <QIODevice>
#include <QTime>
#include <QThread>
#include <QByteArray>
#include <QLocalSocket>
#include <QAbstractSocket>
#include <QWeakPointer>

#include "tasconstants.h"
#include "tascommand.h"
#include "tascoreutils.h"

static const int READ_TIME_OUT = 10000;

class TasMessage;
class TasSocket;
class TasSocketWriter;
class TasSocketReader;

/*
  Only one servicehandler for requests and responses is allowed so 
  using interfaces and observer type approach instead of signals.
 */

class RequestHandler
{
public:
  virtual ~RequestHandler(){};
  virtual void serviceRequest(TasMessage& request, TasSocket* requester) = 0;
};

class ResponseHandler
{
public:
  virtual ~ResponseHandler(){};
  virtual void serviceResponse(TasMessage& response) = 0;
};

class TasSocket : public QObject
{
    Q_OBJECT
    
public:

    TasSocket(QIODevice* device, QObject *parent = 0 );       
    ~TasSocket();

	virtual void closeConnection() = 0;

    bool sendRequest(const qint32& messageId, const QString& message);
    bool sendRequest(const qint32& messageId, const QByteArray& message);
    bool sendResponse(const qint32& messageId, const QString& message, bool compressed = false);
    bool sendResponse(const qint32& messageId, const QByteArray& message, bool compressed = false);
    bool sendError(const qint32& messageId, const QString& message);
    bool sendError(const qint32& messageId, const QByteArray& message, bool compressed = false);
	
    bool syncRequest(const qint32& messageId, const QByteArray& requestMessage, TasMessage &reply);
    bool syncRequest(const qint32& messageId, const QString& requestMessage, TasMessage &reply);
    

	bool sendMessage(TasMessage& message);

	void setRequestHandler(RequestHandler* handler);
	void setResponseHandler(ResponseHandler* handler);
	void clearHandlers();

signals:
	void socketClosed();
	void messageSent();

public slots:
	virtual void disconnected() = 0;
	void messageAvailable(TasMessage& message);
    void cleanUp(QObject* obj = 0);

protected:
	void closeDevice();

private:
    TasSocketReader* mReader;
    TasSocketWriter* mWriter;
	QIODevice* mDevice;
	ResponseHandler* mResponseHandler;
	RequestHandler* mRequestHandler;
};

class TasSocketWriter : public QObject
{
    Q_OBJECT

public:
   TasSocketWriter(QIODevice* device, QObject* parent=0);
	~TasSocketWriter();
	bool isWritable();
	bool writeMessage(TasMessage& message);
	bool writeData(const qint32& messageId, const QByteArray& message, QString& errorMessage, int timeout);
    void close();

private:
	void writeBytes(const QByteArray& msgBytes);
	QByteArray makeHeader(TasMessage& message);
	void flush();

private:
	QIODevice* mDevice;
	QAbstractSocket* mTcpSocket;
	QLocalSocket* mLocalSocket;
};


class TasSocketReader : public QObject
{
    Q_OBJECT
public:
    TasSocketReader(QIODevice* device, QObject* parent=0);
	~TasSocketReader();

    bool readOneMessage(TasMessage& message);
    void close();

signals:
	void messageRead(TasMessage& message);

private slots:
	void readMessageData();

private:
	QIODevice* mDevice;
};


class TAS_EXPORT TasServerSocket : public TasSocket
{
    Q_OBJECT
public:
    TasServerSocket(QIODevice* device, QObject *parent = 0 );       

    void closeConnection();
	
	void setIdentification(const QString& identification);
	
public slots:
    void disconnected();

private:
	QString mIdentification;
};

class TAS_EXPORT TasClientSocket : public TasSocket
{
    Q_OBJECT
public:
  TasClientSocket(QIODevice* device, QObject *parent = 0 );       
  void closeConnection();

public slots:
  void disconnected();

};

#endif
