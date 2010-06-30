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
 

#include "taslogger.h"                
#include "tasdatashare.h"
#include "tasconstants.h"

/*!
    \class TasDataShare
    \brief TasDataShare stores data to be shared between qttasserver and plugin processes.
    
 */

/*!
    Construct a new TasDataShare.
*/
TasDataShare::TasDataShare()
{ 
}

/*!
    Destructor
*/
TasDataShare::~TasDataShare()
{    
    QMutableHashIterator<QString, QSharedMemory*> i(mStoredDataBlocks);
    while (i.hasNext()) {
        i.next();
        QSharedMemory* mem = i.value();
        mem->detach();
        delete mem;
    }
    mStoredDataBlocks.clear();
}

bool TasDataShare::storeSharedData(const QString& identifier, const TasSharedData& data)
{
    QSharedMemory* storage = new QSharedMemory(identifier);
    QByteArray array = data.asArray();
    int size = array.size();
    if(!storage->create(size)){
        delete storage;
        return false;
    }

    storage->lock();
    char *to = (char*)storage->data();
    const char *from = array.data();
    memcpy(to, from, qMin(storage->size(), size));
    storage->unlock();
    mStoredDataBlocks.insert(identifier, storage);
    return true;
}

TasSharedData* TasDataShare::loadSharedData(const QString& identifier)
{
    QSharedMemory storage(identifier);
    if(!storage.attach()){
        return 0;
    }
    storage.lock();
    QByteArray array((char*)storage.constData(), storage.size());
    TasSharedData* sharedData = new TasSharedData(QString::fromAscii((array.data())));
    storage.unlock();
    storage.detach();
    return sharedData;
}

bool TasDataShare::detachSharedData(const QString& identifier)
{
    if(mStoredDataBlocks.contains(identifier)){
        QSharedMemory* mem = mStoredDataBlocks.value(identifier);
        if(!mem->detach()){
            return false;
        }
        delete mem;
        mem = 0;
        mStoredDataBlocks.remove(identifier);
    }
    return true;
}

TasSharedData::TasSharedData(QStringList eventList, QStringList signalList)
{
    mEvents = eventList;
    mSignals = signalList;
    mCreationTime = QDateTime::currentDateTime();
}

TasSharedData::TasSharedData(QString rawData)
{
    QStringList complete = rawData.split(";");
    mCreationTime = QDateTime::fromString(complete.at(0), DATE_FORMAT);
    mEvents = complete.at(1).split(",",QString::SkipEmptyParts);
    mSignals = complete.at(2).split(",",QString::SkipEmptyParts);
}

TasSharedData::~TasSharedData()
{}

QByteArray TasSharedData::asArray() const
{
    QString data = mCreationTime.toString(DATE_FORMAT) + ";" + mEvents.join(",") + ";" + mSignals.join(",");
    return data.toAscii();
}

QStringList TasSharedData::eventsToListen()
{
    return mEvents;
}

QStringList TasSharedData::signalsToListen()
{
    return mSignals;
}

QDateTime TasSharedData::creationTime()
{
    return mCreationTime;
}
