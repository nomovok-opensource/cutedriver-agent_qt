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
 


#ifndef TASMESSAGES_H
#define TASMESSAGES_H

#include "tasconstants.h"

#include <QByteArray>

class TasSocket;

class TAS_EXPORT TasMessage
{
 public:
    TasMessage();
    TasMessage(quint8 flag, bool compressed, QByteArray* data, qint32 messageId);
    ~TasMessage();

	void setData(QString message);
	void setFlag(quint8 flag);
	void setIsCompressed(bool compressed);
	void setTimedOut(bool timedOut);
	void setMessageId(qint32 messageId);
	void setIsError(bool isError);
	void setErrorMessage(QString message);
	void setData(QByteArray* data, bool isCompressed = false);
	bool isError();

	QByteArray* data();	
	QByteArray* dataCompressed();	
	QString dataAsString();
	quint8 flag() const;
    bool isCompressed() const;
	qint32 messageId();

private:
	void uncompressData();
	
private:
	QByteArray* mData;
	quint8 mFlag;
    bool mCompressed;
	qint32 mMessageId;
	bool mIsError;
};

class TAS_EXPORT TasResponse : public TasMessage
{
public:

    TasResponse(qint32 messageId, QByteArray* data=0, bool isCompressed = false, bool isError = false);
	~TasResponse();

	void setRequester(TasSocket* socket);
	TasSocket* requester();


private:
	TasSocket* mSocket;
};

class TAS_EXPORT ResponseFilter
{
public:
    virtual ~ResponseFilter(){};
	virtual void filterResponse(TasMessage& response) = 0;
};


#endif
