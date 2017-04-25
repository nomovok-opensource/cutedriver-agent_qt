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


#ifndef POPUPFIXTUREPLUGIN_H
#define POPUPFIXTUREPLUGIN_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include <tasqtfixtureplugininterface.h>
#include <tasqtdatamodel.h>
#include <tasconstants.h>
#include <tasuitraverser.h>

class PopupFixture : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.nokia.testability.PopupFixture" FILE "popupfixture.json")
    Q_INTERFACES(TasFixturePluginInterface)

public:
    PopupFixture(QObject* parent=0);
    ~PopupFixture();
    bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);
    bool eventFilter(QObject *target, QEvent *event);

private slots:
    void traverse(const QString& className);

private:
    void startPopupDetection(const QString& className, int interval);


private:
    QHash<QString,int> mClassNames;
    QHash<QString, TasDataModel*> mPopupData;
    bool mFiltering;
    TasUiTraverser* mTraverser;
 };

class PopupTimer : public QObject
{
    Q_OBJECT

public:
  PopupTimer(const QString className, int interval);

signals:
    void traverseState(const QString&);

private slots:
    void timeout();

private:
    QString mClassName;
};

#endif

