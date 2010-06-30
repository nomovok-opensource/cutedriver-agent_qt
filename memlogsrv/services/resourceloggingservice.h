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
 

#ifndef RESOURCELOGGINGSERVICE_H_
#define RESOURCELOGGINGSERVICE_H_

#include <tascommand.h>
#include <tasconstants.h>
#include <QTextStream>

class QTimer;
class QFile;
class ResourceLoggingTimer;
class ResourceDataGatherer;

class ResourceLoggingService : public QObject, public TasServiceCommand
{
    Q_OBJECT

public:
    enum ResourceType {
        TypeNone,
        TypeDeviceMem,
        TypeProcessMem
    };
    
public:
    ResourceLoggingService();
    ~ResourceLoggingService();
    bool executeService(TasCommandModel& model, TasResponse& response);
    QString serviceName() const { return RESOURCE_LOGGING_SERVICE; }
    
public slots:
    void timerError(int errorCode, QString& resourceIdentifier);

private:
    int startLogging(
            ResourceLoggingService::ResourceType resourceType, 
            TasCommand& command);
    int stopLogging(
            ResourceLoggingService::ResourceType resourceType, 
            TasCommand& command, 
            QString& responseData);
    bool commandIs(
            TasCommandModel& commandModel, 
            const QString& commandName, 
            TasCommand*& command);

private:
    QList<ResourceLoggingTimer*> mLoggingTimers;
    bool mResourceLoggerProcessRunning;
   
};

class ResourceLoggingTimer : public QObject
{
    Q_OBJECT

public:
    ResourceLoggingTimer(
            ResourceLoggingService::ResourceType resourceType, 
            QString& resourceIdentifier, 
            QString& logFileName, 
            bool timestampAbsolute, 
            int milliSecInterval);
    ~ResourceLoggingTimer();
    int interval();
    void start();
    void start(int milliSecInterval);
    void stop();
    const QString& resource() { return mResourceIdentifier; }
    const QString& getLogFileName() { return mLogFileName; }
    QString* getLogFileData();
    
signals:
    void error(int errorCode, QString& resourceIdentifier);
    
public slots:
    void run();

private:
    QTimer* mTimer;
    ResourceDataGatherer* mDataGatherer;
    QFile* mLogFile;
    QString* mLogFileData;
    QTextStream mLogFileStream;
    ResourceLoggingService::ResourceType mResourceType;
    QString mResourceIdentifier;
    QString mLogFileName;
    int mMilliSecInterval;
    bool mFirstLogFileWrite;
    
};

#endif /* RESOURCELOGGINGSERVICE_H_ */
