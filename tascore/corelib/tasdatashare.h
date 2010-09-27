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
 


#ifndef TASDATASHARE_H
#define TASDATASHARE_H

#include <QSharedMemory>
#include <QStringList>
#include <QDateTime>
#include <QHash>

#include "tasconstants.h"

class TasSharedData;

class TasDataShare
{
   
public:
    TasDataShare();       
    ~TasDataShare();

	bool storeSharedData(const QString& identifier, const TasSharedData& data);
	TasSharedData* loadSharedData(const QString& identifier);
	bool detachSharedData(const QString& identifier);
	 
private:
	QHash<QString, QSharedMemory*> mStoredDataBlocks;
};

class TAS_EXPORT TasSharedData
{
public:
    TasSharedData(QStringList eventList, QStringList signalList);
    ~TasSharedData();

	QStringList eventsToListen();
	QStringList signalsToListen();
	QByteArray asArray() const;	  
	QDateTime creationTime();

private:
    TasSharedData(QString rawData);

private:
	QStringList mEvents;
	QStringList mSignals;
	QDateTime mCreationTime;
	friend class TasDataShare;
};


#endif
