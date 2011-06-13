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

#ifndef APPLICATIONINSTANCE_H
#define APPLICATIONINSTANCE_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QStringList>

class CucumberApplicationManager : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void verifySelectedApp(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    Q_INVOKABLE void selectApp(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    Q_INVOKABLE void attachApp(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    Q_INVOKABLE void startApp(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    Q_INVOKABLE void startAppTable(const QString &regExpPattern, const QVariantList &args, QObject *sender);


    explicit CucumberApplicationManager(QObject *parent = 0);

    void registerSteps(QObject *registrarObject, const char *method);
    // method is of type:
    // void method(const QRegExp &regExp, QObject *object, const char *method, const char *sourceFile, int sourceLine);

    void beginScenario();
    void endScenario();

signals:

private:
    QString doStartApp(const QString &id, const QString &program, const QStringList &arguments);

    QMap<QString, QString> pidMap; // maps application id to it's "pid" as received from StartAppService
    QString currentApplicationId;
    QString workingDirectoryPath;
    QStringList startEnvironment;
};

#endif // APPLICATIONINSTANCE_H
