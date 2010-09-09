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
#include <memspydriverclient.h>
#include <QString>

_LIT( WildCardAny, "*" );

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
{
    if (ResourceLoggingService::TypeProcessMem == mResourceType) {
        if (mMemSpyDriver) {
            if (mDataSourceOpen) {
                mMemSpyDriver->Close();
                mDataSourceOpen = false;
            }
            delete mMemSpyDriver;
        }
    }
}

void ResourceDataGatherer::initializeMemLogging()
{
    TasLogger::logger()->debug("> ResourceDataGatherer::initializeMemLogging");
    int error = KErrNone;
    bool foundAny = false;
    RThread thread;
    
    RBuf match;
    User::LeaveIfError(match.Create(mResourceIdentifier.length() + 2));
    CleanupClosePushL(match);
    match.Append(WildCardAny);
    match.Copy((const TUint16*) mResourceIdentifier.constData(), mResourceIdentifier.length());
    match.Append(WildCardAny);
    TFindThread findThread(match);
    TFullName threadFullName;
    while (findThread.Next(threadFullName) == KErrNone  && !foundAny) {
        foundAny = true;
        error = thread.Open(findThread);
        if (error == KErrNone) {
            //check that the thread is still alive
            if(thread.ExitType() != EExitPending){
                thread.Close();
                TasLogger::logger()->warning("ResourceDataGatherer::initializeMemLogging already exited thread found, continue searching.");
                foundAny = false;
                continue;
            }
            else{                
                mThreadId = thread.Id().Id();
                RProcess parentProcess;
                error = thread.Process(parentProcess);
                if (KErrNone == error) {
                    mParentPid = parentProcess.Id().Id();
                }
                mThreadFullName = QString((const QChar*) threadFullName.Ptr(), threadFullName.Length());
                thread.Close();
            }
        }
    }
        
    if (!foundAny) {
        error = KErrNotFound;
    }

    if (error) {
        TasLogger::logger()->error("  ResourceDataGatherer::initializeMemLogging thread finding error: " + QString::number(error));
        User::Leave(error);
    }

    CleanupStack::PopAndDestroy(); // match
    
    mMemSpyDriver = new (ELeave) RMemSpyDriverClient();
    if (mMemSpyDriver) {
        error = mMemSpyDriver->Open();
        if (KErrNone == error) {
            mDataSourceOpen = true;
        }
        else {
            TasLogger::logger()->error("  ResourceDataGatherer::initializeMemLogging driver opening error: " + QString::number(error));
            User::Leave(error);
        }   
    }
    else {
        TasLogger::logger()->error("  ResourceDataGatherer::initializeMemLogging driver creation error: " + QString::number(error));
        User::Leave(error);
    }
}

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
    if (mMemSpyDriver && mDataSourceOpen) {
        int suspendedCount = mMemSpyDriver->ProcessThreadsSuspend(mParentPid);
        if (suspendedCount > 0) {
            TMemSpyHeapInfo heapInfo;
            //get heap info
            int error = mMemSpyDriver->GetHeapInfoUser(heapInfo, mThreadId);

            //resume always
            int suspendError = mMemSpyDriver->ProcessThreadsResume(mParentPid);
            if(suspendError != KErrNone){
                TasLogger::logger()->error("  ResourceDataGatherer::getMemLoggingData resuming error: " + QString::number(suspendError));
            }

            if (error) {
                TasLogger::logger()->error("  ResourceDataGatherer::getMemLoggingData heap info error: " + QString::number(error));
                return error;
            }

            if (mFirstLogFileWrite) {
                //logEntry.append(mThreadFullName + "\n");
                mTimestamp = 0;
                mTime.start();
                mFirstLogFileWrite = false;
            }
            else {
                mTimestamp += mTime.restart();                
            }
            
            QString timestamp;
            if (mTimestampAbsolute) {
                timestamp = mTime.currentTime().toString();
            }
            else {
                timestamp = QString::number(mTimestamp);
            }
            
            TMemSpyHeapStatisticsRHeap stats = heapInfo.AsRHeap().Statistics();
            TUint totalSize =       heapInfo.AsRHeap().MetaData().iHeapSize;
            TUint allocatedSize =   stats.StatsAllocated().TypeSize();
            TUint freeSize =        stats.StatsFree().TypeSize();
            TUint allocatedCells =  stats.StatsAllocated().TypeCount();
            TUint freeCells =       stats.StatsFree().TypeCount();
            TUint slackSize =       stats.StatsFree().SlackSpaceCellSize();

            // <timestamp> size:<total>,<allocated>,<free> \ 
            //             cells:<total>,<allocated>,<free> \
            //             slack:<size>\\n
            logEntry.append(timestamp);
            logEntry.append(" size:");
            logEntry.append(QString::number(totalSize));
            logEntry.append(",");
            logEntry.append(QString::number(allocatedSize));
            logEntry.append(",");
            logEntry.append(QString::number(freeSize));
            logEntry.append(" cells:");
            logEntry.append(QString::number(allocatedCells + freeCells));
            logEntry.append(",");
            logEntry.append(QString::number(allocatedCells));
            logEntry.append(",");
            logEntry.append(QString::number(freeCells));
            logEntry.append(" slack:");
            logEntry.append(QString::number(slackSize));
            logEntry.append("\n");
            
            return TAS_ERROR_NONE;
        }
        else {
            TasLogger::logger()->error("ResourceDataGatherer::getMemLoggingData could not suspend. ");
            return TAS_ERROR_NOT_FOUND;
        }
    }
    else {
        TasLogger::logger()->error("ResourceDataGatherer::getMemLoggingData driver not ready");
        return TAS_ERROR_INVALID_STATE;
    }
}
