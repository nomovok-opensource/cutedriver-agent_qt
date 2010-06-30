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
 



#ifndef TASLOGGER_H
#define TASLOGGER_H

#include <QFile>
#include <QTime>
#include <QStringList>
#include <QMutex>

#include "tasconstants.h"
#include "tasqtcommandmodel.h"

class EventLogger;

#ifdef Q_OS_SYMBIAN
static const QString LOG_PATH = "\\logs\\testability\\";
#else
static const QString LOG_PATH = "/logs/testability/";
#endif

enum LogType
{
    FATAL = 0,
    ERROR = 1,
    INFO = 2,
    WARNING = 3,
    DEBUG = 4
};

class TAS_EXPORT TasLogger 
{

private:
    TasLogger();
    ~TasLogger();
public:
    static TasLogger* logger();
    void debug(const QString message);
    void info(const QString message);
    void warning(const QString message);
    void error(const QString message);
    void fatal(const QString message);
    void setLevel(const LogType& level);
    void setLogFile(const QString logFileName);
	void setLogDir(const QString logPath);
    void enableLogger();
	void disableLogger();
	void setOutputter(bool intercept=true);
	void logEvents(QStringList filters=QStringList());
	void stopEventLogging();
	void useQDebug(bool use);
	void setLogSize(int size);
	void configureLogger(TasCommand& command);
	void configureEventLogger(TasCommand& command);
	void clearLogFile();
private:
    void writeLogLine(LogType type, const QString& message);    
    void intialize();    
	void outPut(const QString& line);

private:
    static TasLogger* mInstance;    
	bool mUseQDebug;
    QFile* mOut;
    bool mEnabled;
    LogType mCurrentLevel;    
	QString mLogFileName;
	QString mLogPath;
	EventLogger* mEventLogger;
	QTime mLastLogRollCheck;
	int mLogSize;
    QMutex mMutex;
};

/* Helpers
#ifdef __PRETTY_FUNCTION__
#define TASLOG_DEBUG(msg) TasLogger::logger()->debug(QString(__PRETTY_FUNCTION__) + QString(": ") + QString(msg));
#else
#define TASLOG_DEBUG(msg) TasLogger::logger()->debug(QString(__FUNCTION__) + QString(": ") + QString(msg));
#endif
*/

class EventLogger : public QObject
{
  Q_OBJECT

public:
  bool eventFilter(QObject *target, QEvent *event);

  void setFilterStrings(QStringList filterString);

private:
  bool logEvent(QString eventType);

private:
  QStringList mFilterStrings;
  
};

#endif
