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
 

#include "tasmessages.h"
#include "tassocket.h"
#include "taslogger.h"

TasMessage::TasMessage() :
    mFlag(0)
  , mCompressed(false)
  , mMessageId(0)
{
}

TasMessage::TasMessage(quint8 flag, bool compressed, const QByteArray& data, qint32 messageId) :
    mFlag(0)
  , mCompressed(false)
  , mMessageId(messageId)
{
    setFlag(flag);
    setData(data, compressed);
}

TasMessage::~TasMessage()
{
    mData.clear();
}


void TasMessage::setFlag(quint8 flag)
{
    mFlag = flag;
}

void TasMessage::setIsCompressed(bool compressed)
{
    mCompressed = compressed;
}

QByteArray& TasMessage::data()
{
    return mData;
}

QByteArray& TasMessage::dataCompressed()
{
    if(!mCompressed){
        mData = qCompress(mData, 9);
        mCompressed = true;
    }
    return mData;
}

void TasMessage::uncompressData() 
{
    if(mCompressed){
        mData = qUncompress(mData);
        mCompressed = false;
    }
}

QString TasMessage::dataAsString()
{
    uncompressData();
    return QString::fromUtf8(mData.data(), mData.size()).trimmed();
}

quint8 TasMessage::flag() const
{
    return mFlag;
}

bool TasMessage::isCompressed() const
{
    return mCompressed;
}

qint32 TasMessage::messageId()
{
    return mMessageId;
}

void TasMessage::setMessageId(qint32 messageId)
{
    mMessageId = messageId;
}

/*!
  Set data for message. Takes ownership of data.
 */
void TasMessage::setData(const QByteArray& data, bool isCompressed)
{
    mData.clear();
    mData = data;
    setIsCompressed(isCompressed);
}

void TasMessage::setErrorMessage(const QString& message)
{
    setIsError(true);
    setIsCompressed(false);
    setData(message);
}

bool TasMessage::isError() const
{
    return (mFlag == ERROR_MSG);
}

bool TasMessage::isRequest() const
{
    return (mFlag == REQUEST_MSG);
}

bool TasMessage::isResponse() const
{
    return (mFlag == RESPONSE_MSG || mFlag == ERROR_MSG);
}

void TasMessage::setIsError(bool isError)
{
    if(isError){
        setFlag(ERROR_MSG);
    }
    else{
        setFlag(RESPONSE_MSG);
    }
}

void TasMessage::setData(const QString& message)
{
    setData(QByteArray(message.toUtf8()), false);
}


TasResponse::TasResponse(qint32 messageId, const QByteArray& data, bool isError, bool isCompressed)
{
    //defaults to OK_MESSAGE
    if(data.isEmpty()){
        TasMessage::setData(QString(OK_MESSAGE));
    }
    else{
        TasMessage::setData(data, isCompressed);
    }
    TasMessage::setMessageId(messageId);
    TasMessage::setIsError(isError);
    mSocket = 0;
}

TasResponse::~TasResponse()
{}

void TasResponse::setRequester(TasSocket* socket)
{
    mSocket = socket;
}

TasSocket* TasResponse::requester()
{
    return mSocket;
}
