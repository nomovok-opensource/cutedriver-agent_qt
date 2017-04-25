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



#include "resourcedatagatherer.h"
#include <taslogger.h>
#include <QString>

ResourceDataGatherer::ResourceDataGatherer(
        ResourceLoggingService::ResourceType resourceType,
        const QString& resourceIdentifier,
        bool timestampAbsolute) :
            mFirstLogFileWrite(true),
            mTimestampAbsolute(timestampAbsolute),
            mDataSourceOpen(false)
{
    mResourceType = resourceType;
    mResourceIdentifier = resourceIdentifier;

    if (ResourceLoggingService::TypeProcessMem == mResourceType) {
        initializeMemLogging();
    }
}

ResourceDataGatherer::~ResourceDataGatherer()
{}

void ResourceDataGatherer::initializeMemLogging()
{}

int ResourceDataGatherer::getLogEntryData(QString& logEntry)
{
    if (ResourceLoggingService::TypeProcessMem == mResourceType) {
        return getMemLoggingData(logEntry);
    }
    else {
        return TAS_ERROR_INVALID_STATE;
    }
}

int ResourceDataGatherer::getMemLoggingData(QString& logEntry)
{
    Q_UNUSED(logEntry);
    return TAS_ERROR_NOT_IMPLEMENTED;
}
