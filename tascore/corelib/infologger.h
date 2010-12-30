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
 


#ifndef INFOLOGGER_H
#define INFOLOGGER_H

#include <QTimer>
#include <QDateTime>
#include <QTime>
#include <QFile>

#include "tasconstants.h"
#include "tasqtdatamodel.h"
#include "tasqtcommandmodel.h"
#include "tasdeviceutils.h"
#include "tasmessages.h"

class InfoLogger : public QObject
{
    Q_OBJECT
public:

    enum LoggingState 
	{
	  CpuLogging  = 0x01,
	  MemLogging  = 0x02,
	  GpuLogging  = 0x04,
	  LoggingMask = 0x07
	};
    Q_DECLARE_FLAGS(LoggingStates, LoggingState)

    InfoLogger();
    ~InfoLogger();

	void performLogService(TasCommandModel& model, TasResponse& response);

private:

	void checkLoggerState();

	void loadCpuData(TasResponse& response, TasCommand* command);
	void loadGpuData(TasResponse& response, TasCommand* command);
	void loadMemData(TasResponse& response, TasCommand* command);

	void logMem();
	void logCpu();
	void logGpu();


	QByteArray loadData(QFile* file, const QString& name, TasCommand* command);
	bool makeFileName(TasCommand* command, const QString& type, QString& name);

	void writeLine(const QString& line, QFile* file);
	QFile* openFile(const QString& fileName, TasCommand* command);

private slots:	
	void timerEvent();

private:
	QTimer mTimer;
	LoggingStates mState;
	QFile* mCpu;
	QFile* mMem;
	QFile* mGpu;
	QTime mInterval;
	qreal mLastCpuTime;
	TasDeviceUtils* mDeviceUtils;
};

#endif
