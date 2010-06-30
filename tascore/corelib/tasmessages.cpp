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

TasMessage::TasMessage()
{
    mData = 0;
    mCompressed = false;
}

TasMessage::TasMessage(quint8 flag, bool compressed, QByteArray* data, qint32 messageId)
{
    mData = 0;
    mCompressed = false;
    mMessageId = messageId;
    setFlag(flag);
    setData(data, compressed);
}

TasMessage::~TasMessage()
{
    if(mData){
        delete mData;
        mData = 0;
    }
}


void TasMessage::setFlag(quint8 flag)
{
    mFlag = flag;
}

void TasMessage::setIsCompressed(bool compressed)
{
    mCompressed = compressed;
}

QByteArray* TasMessage::data()
{
    return mData;
}

QByteArray* TasMessage::dataCompressed()
{
    if(!mCompressed){
        QByteArray temp = qCompress(*mData, 9);
        mData->clear();
        mData->append(temp);
        mCompressed = true;
    }
    return mData;
}

void TasMessage::uncompressData() 
{
    if(mCompressed){
        QByteArray temp = qUncompress(*mData);
        mData->clear();
        mData->append(temp);
        mCompressed = false;
    }
}

QString TasMessage::dataAsString()
{
    uncompressData();
    return QString::fromUtf8(mData->data(), mData->size()).trimmed();
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
void TasMessage::setData(QByteArray* data, bool isCompressed)
{
    if(mData){
        delete mData;
    }
    mData = data;
    setIsCompressed(isCompressed);
}

void TasMessage::setErrorMessage(QString message)
{
    setIsError(true);
    setIsCompressed(false);
    setData(message);
}

bool TasMessage::isError()
{
    return mIsError;
}

void TasMessage::setIsError(bool isError)
{
    mIsError = isError;
    if(mIsError){
        setFlag(ERROR_MSG);
    }
    else{
        setFlag(RESPONSE_MSG);
    }
}

void TasMessage::setData(QString message)
{
    setData(new QByteArray(message.toUtf8()), false);
}


TasResponse::TasResponse(qint32 messageId, QByteArray* data, bool isError, bool isCompressed)
{
    //defaults to OK_MESSAGE
    if(!data){
        data = new QByteArray(OK_MESSAGE.toUtf8());
    }
    TasMessage::setMessageId(messageId);
    TasMessage::setData(data, isCompressed);
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
