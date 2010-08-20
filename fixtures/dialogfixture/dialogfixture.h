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
 

#ifndef DIALOGFIXTUREPLUGIN_H
#define DIALOGFIXTUREPLUGIN_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include <tasqtfixtureplugininterface.h>
#include <tasqtdatamodel.h>
#include <tasconstants.h>
#include <tasuitraverser.h>

class DialogFixture : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT
    Q_INTERFACES(TasFixturePluginInterface)
 
public:
    DialogFixture(QObject* parent=0);
    ~DialogFixture();
    bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);
    bool eventFilter(QObject *target, QEvent *event);

private slots:
	void traverse(const QString& className);

private:
	void startDialogDetection(const QString& className, int interval);


private:
	QHash<QString,int> mClassNames;
	QHash<QString, TasDataModel*> mDialogData;
	bool mFiltering;
	TasUiTraverser* mTraverser;
 };

class DialogTimer : public QObject
{
    Q_OBJECT

public:
  DialogTimer(const QString className, int interval);

signals:	
	void traverseState(const QString&);

private slots:
	void timeout();

private:
	QString mClassName;
};

#endif
 
