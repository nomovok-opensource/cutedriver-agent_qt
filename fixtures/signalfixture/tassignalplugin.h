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
 


#ifndef TASSIGNALFIXTUREPLUGIN_H
#define TASSIGNALFIXTUREPLUGIN_H

#include <QObject>
#include <QWidget>
#include <QTime>
#include <QSignalSpy>
#include <tasqtfixtureplugininterface.h>
#include <tassignalspy.h>
#include <tasqtdatamodel.h>

class TasSignalPlugin : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.nokia.testability.TasSignal" FILE "tassignal.json")
    Q_INTERFACES(TasFixturePluginInterface)
 
public:
     TasSignalPlugin(QObject* parent=0);
     ~TasSignalPlugin();
     bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);
 
 	 
private:
	
	 bool listSignals(void *objectInstance, QString ptrType, QString & stdOut);
	 bool enableSignal(void *objectInstance, QHash<QString, QString> parameters, QString & stdOut);
	 QObject* castToObject(void* objectInstance, QString ptrType);
	 bool printSignals(QString& stdOut);
	 void printErrorMsg(QHash<QString, QString> parameters, QString& stdOut);
	 bool clearSignals(QString& stdOut);


private:
	 TasDataModel* mOccuredSignals;
	 QHash<QString, TasSignalSpy*> spyContainerHash;
};

#endif

