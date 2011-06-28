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
#include <tasqtfixtureplugininterface.h>
#include <taslogger.h>
#include <startappservice.h>


#include <QtCore>

#include <qjson_warpper.h>


#define DP "CucumberWireprotocolServer" << __FUNCTION__
#define DPL "CucumberWireprotocolServer" << QString("%1:%2").arg(__FUNCTION__).arg(__LINE__)





CucumberApplicationManager::CucumberApplicationManager(QObject *parent) :
    QObject(parent)
  , retryTimeout(4*1000)
  , retryInterval(500)
  , retryTimer(new QTimer(this))
  , pendingSender(0)
  , pendingTimer(new QTimer(this))
{
    retryTimer->setInterval(retryInterval);
    connect(retryTimer, SIGNAL(timeout()), SLOT(doRetryTimer()));

    pendingTimer->setSingleShot(true);
    pendingTimer->setInterval(120*1000);
    connect(pendingTimer, SIGNAL(timeout()), SLOT(pendingSenderTimeout()));
}




void CucumberApplicationManager::pendingSenderTimeout()
{
    if (pendingSender) {
        QMetaObject::invokeMethod(pendingSender, "cucumberFailResponse", Qt::QueuedConnection,
                                  Q_ARG(QString, "Fallback timeout waiting for response from application!"));
        pendingSender = 0;
    }
}


void CucumberApplicationManager::handleFixtureResult(bool success, const QString &text, quintptr callId)
{
    qDebug() << DPL << success << text << callId;
    if (!callId || callId != reinterpret_cast<quintptr>(pendingSender)) {
        TasLogger::logger()->error("CucumberApplicationManager::qtScriptResult called with invalid callId");
    }
    else {
        pendingTimer->stop();

        bool invokeOk;

        if (success) {
            invokeOk = QMetaObject::invokeMethod(pendingSender, "cucumberSuccessResponse", Qt::QueuedConnection);
        }
        else {
            invokeOk = QMetaObject::invokeMethod(pendingSender, "cucumberFailResponse", Qt::QueuedConnection,
                                             Q_ARG(QString, text));
        }
        qDebug() << DPL << success << "sender invokation" << invokeOk;
        pendingSender = 0;
    }
}



static const char *regExp_callScriptMethod = "I call (\\S+) on application object (\\S+)";
static const int line_callScriptMethod = __LINE__ + 1;
void CucumberApplicationManager::callScriptMethod(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    if (pendingSender) {
        qDebug() << DPL << "ALREADY HAD pendingSender!";
        pendingTimer->stop();
        pendingSenderTimeout();
    }

    if (args.size() != 2) {
        invokePlainSender(sender, "Expected 2 step arguments");
    }
    else {
        TasClientManager *clientManager = TasClientManager::instance();
        QString processId = pidMap[currentApplicationId];
        qDebug() << DPL << "looking for" << currentApplicationId << "pid" << processId;
        TasClient *client = clientManager->findByProcessId(processId);

        if (!client) {
            QString errorMsg = QString("Application '%1' not found").arg(currentApplicationId);
            doReplyOrRetry(&CucumberApplicationManager::callScriptMethod, errorMsg, regExpPattern, args, sender);
        }
        else {
            QByteArray jsonData = QJson::Serializer().serialize(args);
            QHash<QString, QString> argMap;
            argMap["qjson"] = jsonData;
            argMap["regExp"] = regExpPattern;
            qDebug() << DPL << argMap;

            retryData.clear();
            retryTimer->stop();

            pendingSender = sender;
            pendingTimer->start();
            client->callFixture(this, "handleFixtureResult", reinterpret_cast<quintptr>(pendingSender),
                                "qtscriptfixture", CUCUMBER_STEP_ACTION, argMap);
            // callFixture will call handleFixtureResult, which will send response
        }
    }
}


static const char *regExp_checkScriptProperty = "application object (\\S+) has (\\S+) ['\"](\\S+)['\"]";
static const int line_checkScriptProperty = __LINE__ + 1;
void CucumberApplicationManager::checkScriptProperty(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    if (pendingSender) {
        qDebug() << DPL << "ALREADY HAD pendingSender!";
        pendingTimer->stop();
        pendingSenderTimeout();
    }

    if (args.size() != 3) {
        invokePlainSender(sender, "Expected 3 step arguments");
    }
    else {
        TasClientManager *clientManager = TasClientManager::instance();
        QString processId = pidMap[currentApplicationId];
        qDebug() << DPL << "looking for" << currentApplicationId << "pid" << processId;
        TasClient *client = clientManager->findByProcessId(processId);

        if (!client) {
            QString errorMsg = QString("Application '%1' not found").arg(currentApplicationId);
            doReplyOrRetry(&CucumberApplicationManager::checkScriptProperty, errorMsg, regExpPattern, args, sender);
        }
        else {
            QByteArray jsonData = QJson::Serializer().serialize(args);
            QHash<QString, QString> argMap;
            argMap["qjson"] = jsonData;
            argMap["regExp"] = regExpPattern;
            qDebug() << DPL << argMap;

            retryData.clear();
            retryTimer->stop();

            pendingSender = sender;
            pendingTimer->start();
            client->callFixture(this, "handleFixtureResult", reinterpret_cast<quintptr>(pendingSender),
                                "qtscriptfixture", CUCUMBER_STEP_ACTION, argMap);
            // callFixture will call handleFixtureResult, which will send response
        }
    }

}


static const char *regExp_selectApp = "I select application ?(\\w*)";
static const int line_selectApp = __LINE__ + 1;
void CucumberApplicationManager::selectApp(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    if (args.size() != 1) {
        invokePlainSender(sender, "Expected single application id");
    }
    else {
        QString errorMsg;
        QString id = args.at(0).toString();
        if (!pidMap.contains(id)) {
            invokePlainSender(sender,
                              id.isEmpty() ? QString("No default application")
                                           : QString("No application with id '%1' found").arg(id));
        }
        else {
            // success, errorMsg is empty
            currentApplicationId = id;
            doReplyOrRetry(&CucumberApplicationManager::selectApp, errorMsg, regExpPattern, args, sender);
        }
    }
}


static const char *regExp_attachApp = "I find application ?(\\w*) running with name\\s+(.+)";
static const int line_attachApp = __LINE__ + 1;
void CucumberApplicationManager::attachApp(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    QString errorMsg;
    if (args.size() != 2) {
        invokePlainSender(sender, "Expected optional id and a single application name");
    }
    else {
        QString id = args.at(0).toString();

        if (pidMap.contains(id)) {
            invokePlainSender(sender,
                              id.isEmpty() ? QString("Default application already exists")
                                           : QString("Application with id '%1' already exists").arg(id));
        }
        else {
            // not in pidMap, wait for registration
            TasClientManager *clientManager = TasClientManager::instance();
            QString processId = pidMap.value(id);
            QString name = args.at(1).toString();
            TasClient *client = clientManager->findByApplicationName(name);
            if (client) {
                pidMap.insert(id, client->processId());
                invokePlainSender(sender, QString());
            }
            else {
                QString errorMsg = QString("Application with name '%1' not registered").arg(name);
                doReplyOrRetry(&CucumberApplicationManager::attachApp, errorMsg, regExpPattern, args, sender);
            }
        }
    }
}


static const char *regExp_startApp = "I launch application ?(\\w*) with command\\s+(.+)";
static const int line_startApp = __LINE__ + 1;
void CucumberApplicationManager::startApp(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    if (args.size() != 2) {
        invokePlainSender(sender, QString("Expected optional id and a single command string"));
    }
    else {
        QString id = args.at(0).toString();
        QStringList arguments = args.at(1).toString().split(QRegExp("\\s+"));
        QString program = arguments.takeFirst();

        qDebug() << DP << args << "->" << id << program << arguments;
        QString errorMsg = doStartOrWaitApp(id, program, arguments);
        doReplyOrRetry(&CucumberApplicationManager::startApp, errorMsg, regExpPattern, args, sender);
    }
}


static const char *regExp_startAppTable = "I launch application ?(\\w*) with command:";
static const int line_startAppTable = __LINE__ + 1;
void CucumberApplicationManager::startAppTable(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    if (args.size() != 2) {
        invokePlainSender(sender, QString("Expected optional id and then a list with command and arguments"));

    }
    else {
        QString id = args.at(0).toString();
        QStringList arguments;

        QVariantList rowList = args.at(1).toList();
        foreach(QVariant row, rowList) {
            foreach(QVariant item, row.toList()) {
                //qDebug() << DPL << "--------------" << item;
                arguments << item.toString();
            }
        }

        QString errorMsg;
        if (arguments.size() < 1) {
            errorMsg = QString("Expected command and optional list of arguments");
        }
        else {
            QString program = arguments.takeFirst();

            qDebug() << DP << args << "->" << id << program << arguments;
            errorMsg = doStartOrWaitApp(id, program, arguments);
        }
        doReplyOrRetry(&CucumberApplicationManager::startAppTable, errorMsg, regExpPattern, args, sender);
    }
}


QString CucumberApplicationManager::doStartOrWaitApp(const QString &id, const QString &program, const QStringList &arguments)
{
    if (pidMap.contains(id)) {
        // already started, wait for it to register
        TasClientManager *clientManager = TasClientManager::instance();
        QString processId = pidMap.value(id);
        TasClient *client = clientManager->findByProcessId(processId);
        if (client) return QString(); // success
        else return QString("Application with process id " + processId + " did not register");
    }
    else {
        QString workingDirectory = workingDirectoryPath.isEmpty() ? QDir::currentPath()
                                                                  : workingDirectoryPath;
        QString responseData, responseErrorMessage;
        StartAppService::launchDetached(program, arguments, startEnvironment, workingDirectory, responseData, responseErrorMessage);
        qDebug() << DPL << "->" << program << arguments << '@' << workingDirectory  <<":" << responseData << responseErrorMessage;

        //bool result = QProcess::startDetached(program, arguments, workingDirectory, &pid);
        //qDebug() << DPL << "->" << program << arguments << '@' << workingDirectory  <<":" << result << pid;

        if (responseErrorMessage.isEmpty() && !responseData.isEmpty()) {
            pidMap.insert(id, responseData);
            currentApplicationId = id;
            return QString("Waiting for process id " + responseData + " to register");
        }
        else {
            return QString("Command '%1 %2' gave StartAppService error: %3").arg(program, arguments.join(" "), responseErrorMessage);
        }
    }
}




#define STEPINFO(NAME) { #NAME, regExp_##NAME, line_##NAME }

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
        STEPINFO(callScriptMethod),
        STEPINFO(checkScriptProperty),
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


void CucumberApplicationManager::beginScenario()
{
    pidMap.clear();
}


void CucumberApplicationManager::endScenario()
{
    TasClientManager *clientManager = TasClientManager::instance();
    //    TasClient *client = clientManager->findByApplicationName(name);

    // avoid killing a process twice by converting to set of processId values
    QSet<QString> pidSet = QSet<QString>::fromList(pidMap.values());
    for(QSet<QString>::iterator it = pidSet.begin(); it != pidSet.end(); ++it) {
        QString processId = *it;
        TasLogger::logger()->debug("CucumberApplicationManager::endScenario removing client pid " + processId);
        clientManager->removeClient(*it, true);
    }
    pidMap.clear();
}


void CucumberApplicationManager::doRetryTimer()
{
    qDebug() << DPL << "entry";
    if (retryData.stepFn) {
        // call member function of this object through pointer retryData.stepFn
        ((this)->*(retryData.stepFn))(retryData.regExpPattern, retryData.args, retryData.sender.data());
    }
    else {
        qWarning() << DPL << "called with NULL stepFn, stopping timer and clearing retryData";
        retryTimer->stop();
        retryData.clear();
    }
}


void CucumberApplicationManager::doReplyOrRetry(InvokableStepFn fn, const QString &errorString,
                                                const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    qDebug() << DPL << "entry";
    if (retryData.hasCallback() && !retryData.equals(fn, regExpPattern, args, sender)) {
        qCritical() << DPL << "new updateRetries called when previous not finished!";
        invokePlainSender(retryData.sender.data(), "Cancelled, because of getting a new request!");
    }

    if (errorString.isEmpty()) {
        // success!
        invokePlainSender(sender, errorString);
    }
    else {
        if (retryData.stepFn == NULL) {
            // init retry data
            // first try was already done at this point
            retryData.retriesLeft = retryTimeout / retryTimer->interval();
            retryData.stepFn = fn;
            retryData.regExpPattern = regExpPattern;
            retryData.args = args;
            retryData.sender = sender;
            retryTimer->start();
        }
        else {
            // retryData was tested above, so is guaranteed to match method arguments
            // errorString was tested above, so is guaranteed to be non-empty

            // first try and minimum one retry done at his point
            --retryData.retriesLeft;
            if (retryData.retriesLeft > 0) {
                if (!retryTimer->isActive()) {
                    qWarning() << DPL << "retryTimer was not active on update, starting...";
                    retryTimer->start();
                }
                qDebug() << DPL << "retry after error:" << errorString;
            }
            else {
                // give up
                invokePlainSender(sender, errorString);
            }
        }
    }

}


bool CucumberApplicationManager::invokePlainSender(QObject *sender, const QString &errorMsg)
{
    retryData.clear();
    retryTimer->stop();

    if (!sender) {
        return false;
    }
    else {
        if (errorMsg.isEmpty()) {
            return QMetaObject::invokeMethod(sender, "cucumberSuccessResponse", Qt::QueuedConnection);
        }
        else {
            return QMetaObject::invokeMethod(sender, "cucumberFailResponse", Qt::QueuedConnection,
                                             Q_ARG(QString, errorMsg));
        }
    }
}
