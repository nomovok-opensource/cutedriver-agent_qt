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

#ifndef TASPROPERTYLOGGER_H
#define TASPROPERTYLOGGER_H

#include <QObject>
#include <QTimer>
#include <QPair>
#include <QList>
#include <QFile>
#include <QHash>
#include <infologger.h>

class TasPropertyLogger : QObject
{
  Q_OBJECT
public:
  TasPropertyLogger();
  ~TasPropertyLogger();

  bool startPropertyLog(QObject* object, QHash<QString, QString> params, QString &errorMsg);
  bool getLogData(QObject* object, QHash<QString, QString> params, QString& data);
  void stopLogger(QObject* object, QHash<QString, QString> params);

private slots:
  void logTimerEvent();

private:
  bool validateParams(QHash<QString, QString> params, QString& errorMsg) ;

private:
  QTimer mTimer;
  QHash<QObject*, QHash<QString,QFile*> > mTargets;
  TasInfoLoggerUtil mLoggerUtil;
  int mInterval;
};

#endif


