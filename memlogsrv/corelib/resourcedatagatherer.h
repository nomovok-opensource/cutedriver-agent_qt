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
 
 

#ifndef RESOURCEDATAGATHERER_H_
#define RESOURCEDATAGATHERER_H_

#include "resourceloggingservice.h"
#include <tasconstants.h>
#include <QObject>
#include <QTime>

#ifdef Q_OS_SYMBIAN
class RMemSpyDriverClient;
#endif

class ResourceDataGatherer : public QObject
{
    Q_OBJECT
public:
    ResourceDataGatherer(
            ResourceLoggingService::ResourceType resourceType, 
            const QString& resourceIdentifier,
            bool timestampAbsolute);
    ~ResourceDataGatherer();
    int getLogEntryData(QString& logEntry);

private:
    void initializeMemLogging();
    int getMemLoggingData(QString& logEntry);

private:
    ResourceLoggingService::ResourceType mResourceType;
    QString mResourceIdentifier;
    QString mThreadFullName;
    QTime mTime;
    unsigned long mTimestamp;
    unsigned long mThreadId;
    unsigned long mParentPid;
    bool mFirstLogFileWrite;
    bool mTimestampAbsolute;
    bool mDataSourceOpen;

private: // Platform specific data
#ifdef Q_OS_SYMBIAN
    RMemSpyDriverClient* mMemSpyDriver;    
#endif
    
};

#endif /* RESOURCEDATAGATHERER_H_ */
