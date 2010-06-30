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
 


#ifndef FPSFIXTUREPLUGIN_H
#define FPSFIXTUREPLUGIN_H

#include <QObject>
#include <QHash>
#include <QTime>
#include <QPair>
#include <tasqtfixtureplugininterface.h>

class FspMeasurer;

class FpsFixture : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT
    Q_INTERFACES(TasFixturePluginInterface)
 
public:
     FpsFixture(QObject* parent=0);
     ~FpsFixture();
     bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);

private:
	 void printFpsResults(QList< QPair<QString,int> > fpsData, QObject* target, QString& stdOut);
	 QObject* getTarget(void* objectInstance, QString objectType);

private:
	 QHash<QObject*, FspMeasurer*> mFpsCounters;
 };

class FspMeasurer : public QObject
{
    Q_OBJECT

public:
    FspMeasurer(QObject* target);
    ~FspMeasurer();
    bool eventFilter(QObject *target, QEvent *event);
	void startFpsMeasure();
	void restartFpsMeasure();
	void stopFpsMeasure();
	QList< QPair<QString,int> > collectedData();

private:
	QObject* mTarget;
	int mFpsCounter;
	QTime mTimeStamp;
	QTime mTimer;
	QList< QPair<QString,int> > mFpsValues;

};
#endif
 
