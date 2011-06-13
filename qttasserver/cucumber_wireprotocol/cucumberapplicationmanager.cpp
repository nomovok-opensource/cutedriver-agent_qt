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

#include "cucumberapplicationmanager.h"

#include <tasnativeutils.h>
#include <startappservice.h>

#include <QtCore>


#define DP "CucumberWireprotocolServer" << __FUNCTION__
#define DPL "CucumberWireprotocolServer" << __FUNCTION__ << __LINE__

#define STEPINFO(NAME) { #NAME, regExp_##NAME, line_##NAME }


static const char *regExp_selectApp = "I select application ?(\\w*)";
static const int line_selectApp = __LINE__ + 1;
QString CucumberApplicationManager::selectApp(const QString &/*regExpPattern*/, const QVariantList &args)
{
    if (args.size() != 1) {
        return QString("Expected single application id");
    }

    QString id = args.at(0).toString();
    if (!pidMap.contains(id)) {
        return id.isEmpty() ? QString("No default application")
                            : QString("No application with id '%1' found").arg(id);
    }
    else {
        // success
        currentApplicationId = id;
        return QString();
    }
}


static const char *regExp_attachApp = "I find application ?(\\w*) running with name\\s+(.+)";
static const int line_attachApp = __LINE__ + 1;
QString CucumberApplicationManager::attachApp(const QString &/*regExpPattern*/, const QVariantList &args)
{
    if (args.size() != 2) {
        return QString("Expected optional id and a single application name");
    }

    QString id = args.at(0).toString();

    if (pidMap.contains(id)) {
        return id.isEmpty() ? QString("Default application already exists")
                            : QString("Application with id '%1' already exists").arg(id);
    }

    QString name = args.at(1).toString();

    return QString("Application with name '%1' not found").arg(name);
}


static const char *regExp_startApp = "I launch application ?(\\w*) with command\\s+(.+)";
static const int line_startApp = __LINE__ + 1;
QString CucumberApplicationManager::startApp(const QString &/*regExpPattern*/, const QVariantList &args)
{
    if (args.size() != 2) {
        return QString("Expected optional id and a single command string");
    }

    QString id = args.at(0).toString();
    QStringList arguments = args.at(1).toString().split(QRegExp("\\s+"));
    QString program = arguments.takeFirst();

    qDebug() << DP << args << "->" << id << program << arguments;
    return doStartApp(id, program, arguments);
}


static const char *regExp_startAppTable = "I launch application ?(\\w*) with command:";
static const int line_startAppTable = __LINE__ + 1;
QString CucumberApplicationManager::startAppTable(const QString &/*regExpPattern*/, const QVariantList &args)
{
    if (args.size() != 2) {
        return QString("Expected optional id and then a list with command and arguments");
    }

    QString id = args.at(0).toString();
    QStringList arguments;

    {
        QVariantList rowList = args.at(1).toList();
        foreach(QVariant row, rowList) {
            foreach(QVariant item, row.toList()) {
                qDebug() << DPL << "--------------" << item;
                arguments << item.toString();
            }
        }
    }

    if (arguments.size() < 1) {
        return QString("Expected non-empty command and arguments list");
    }

    QString program = arguments.takeFirst();

    qDebug() << DP << args << "->" << id << program << arguments;
    return doStartApp(id, program, arguments);

}


QString CucumberApplicationManager::doStartApp(const QString &id, const QString &program, const QStringList &arguments)
{
    if (pidMap.contains(id)) {
        return id.isEmpty() ? QString("Default application has already been started")
                            : QString("Application with id '%1' has already been started").arg(id);
    }

    QString workingDirectory = workingDirectoryPath.isEmpty() ? QDir::currentPath()
                                                              : workingDirectoryPath;
    QString responseData, responseErrorMessage;
    StartAppService::launchDetached(program, arguments, startEnvironment, workingDirectory, responseData, responseErrorMessage);
    qDebug() << DPL << "->" << program << arguments << '@' << workingDirectory  <<":" << responseData << responseErrorMessage;

    //bool result = QProcess::startDetached(program, arguments, workingDirectory, &pid);
    //qDebug() << DPL << "->" << program << arguments << '@' << workingDirectory  <<":" << result << pid;

    if (responseErrorMessage.isEmpty() && !responseData.isEmpty()) {
        pidMap.insert(id, responseData);
        return QString();
    }
    else {
        return QString("Command '%1 %2' gave StartAppService error: %3").arg(program, arguments.join(" "), responseErrorMessage);
    }
}


CucumberApplicationManager::CucumberApplicationManager(QObject *parent) :
    QObject(parent)
{
}


void CucumberApplicationManager::registerSteps(QObject *registrarObject, const char *method)
{

    static const struct StepInfo {
        const char *method;
        const char *regExp;
        int line;
    } steps[] = {
        STEPINFO(startApp),
        STEPINFO(startAppTable),
        STEPINFO(attachApp),
        STEPINFO(selectApp),
        { 0, 0, 0 }
    };

    for (int ii=0; steps[ii].method && steps[ii].regExp && steps[ii].line; ++ii) {

        // registering method is of type:
        // void method(const QRegExp &regExp, QObject *object, const char *method, const char *sourceFile, int sourceLine);

        QMetaObject::invokeMethod(registrarObject, method,
                                  Q_ARG(QRegExp, QRegExp(steps[ii].regExp)),
                                  Q_ARG(QObject*, static_cast<QObject*>(this)),
                                  Q_ARG(const char*, steps[ii].method),
                                  Q_ARG(const char*, __FILE__),
                                  Q_ARG(int, steps[ii].line));
    }
}

