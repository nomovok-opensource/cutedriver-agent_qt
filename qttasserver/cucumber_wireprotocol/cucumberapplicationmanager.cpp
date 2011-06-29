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

#include <cucumberstepdata.h>
#include <cucumberutils.h>

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
  , mRetryTimeout(4*1000)
  , mRetryInterval(500)
  , mRetryTimer(new QTimer(this))
  , mPendingSender(0)
  , mPendingTimer(new QTimer(this))
{
    mRetryTimer->setInterval(mRetryInterval);
    connect(mRetryTimer, SIGNAL(timeout()), SLOT(doRetryTimer()));

    mPendingTimer->setSingleShot(true);
    mPendingTimer->setInterval(120*1000);
    connect(mPendingTimer, SIGNAL(timeout()), SLOT(pendingSenderTimeout()));
}


void CucumberApplicationManager::checkPendingSender()
{
    if (mPendingSender) {
        TasLogger::logger()->warning("CucumberApplicationManager::checkPendingSender Unexpectedly found pending sender, cancelling it!");
        mPendingTimer->stop();
        pendingSenderTimeout();
    }

}

void CucumberApplicationManager::pendingSenderTimeout()
{
    if (mPendingSender) {
        QMetaObject::invokeMethod(mPendingSender, "cucumberFailResponse", Qt::QueuedConnection,
                                  Q_ARG(QString, "Fallback timeout waiting for response from application!"));
        mPendingSender = 0;
    }
}


void CucumberApplicationManager::pluginResultCB(bool success, const QString &text, quintptr callId)
{
    qDebug() << DPL << success << text << callId;
    if (!callId || callId != reinterpret_cast<quintptr>(mPendingSender)) {
        TasLogger::logger()->error("CucumberApplicationManager::qtScriptResult called with invalid callId");
    }
    else {
        mPendingTimer->stop();

        bool invokeOk;

        if (success) {
            invokeOk = QMetaObject::invokeMethod(mPendingSender, "cucumberSuccessResponse", Qt::QueuedConnection);
        }
        else {
            invokeOk = QMetaObject::invokeMethod(mPendingSender, "cucumberFailResponse", Qt::QueuedConnection,
                                             Q_ARG(QString, text));
        }
        qDebug() << DPL << success << "sender invokation" << invokeOk;
        mPendingSender = 0;
    }
}


void CucumberApplicationManager::callStepInPlugin(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    checkPendingSender();

    if (!mPluginSteps.contains(regExpPattern)) {
        invokePlainSender(sender, "callStepInPlugin received unexpected step!");
    }
    else {
        TasClientManager *clientManager = TasClientManager::instance();
        QString processId = mPidMap[mCurrentApplicationId];
        qDebug() << DPL << "looking for" << mCurrentApplicationId << "pid" << processId;
        TasClient *client = clientManager->findByProcessId(processId);

        if (!client) {
            QString errorMsg = QString("Application %1 not found").arg(mCurrentApplicationId);
            doReplyOrRetry(&CucumberApplicationManager::callStepInPlugin, errorMsg, regExpPattern, args, sender);
        }
        else {
            QByteArray jsonData = QJson::Serializer().serialize(args);
            QHash<QString, QString> argMap;
            argMap["qjson"] = jsonData;
            argMap["regExp"] = regExpPattern;
            qDebug() << DPL << argMap;

            mRetryData.clear();
            mRetryTimer->stop();

            mPendingSender = sender;
            mPendingTimer->start();
            const CucumberStepData &stepData = mPluginSteps.value(regExpPattern);
            client->callFixture(this, "pluginResultCB", reinterpret_cast<quintptr>(mPendingSender),
                                stepData.flags.value("plugin"), stepData.flags.value("action"), argMap);
            // callFixture will call pluginResultCB, which will send response
        }
    }

}


#if 0
static const char *regExp_callScriptMethod = "I call (\\S+) on application object (\\S+)";
static const int line_callScriptMethod = __LINE__ + 1;
void CucumberApplicationManager::callScriptMethod(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    checkPendingSender();

    if (args.size() != 2) {
        invokePlainSender(sender, "Expected 2 step arguments");
    }
    else {
        TasClientManager *clientManager = TasClientManager::instance();
        QString processId = mPidMap[mCurrentApplicationId];
        qDebug() << DPL << "looking for" << mCurrentApplicationId << "pid" << processId;
        TasClient *client = clientManager->findByProcessId(processId);

        if (!client) {
            QString errorMsg = QString("Application '%1' not found").arg(mCurrentApplicationId);
            doReplyOrRetry(&CucumberApplicationManager::callScriptMethod, errorMsg, regExpPattern, args, sender);
        }
        else {
            QByteArray jsonData = QJson::Serializer().serialize(args);
            QHash<QString, QString> argMap;
            argMap["qjson"] = jsonData;
            argMap["regExp"] = regExpPattern;
            qDebug() << DPL << argMap;

            mRetryData.clear();
            mRetryTimer->stop();

            mPendingSender = sender;
            mPendingTimer->start();
            client->callFixture(this, "pluginResultCB", reinterpret_cast<quintptr>(mPendingSender),
                                "qtscriptfixture", CUCUMBER_STEP_DEFAULTACTION, argMap);
            // callFixture will call pluginResultCB, which will send response
        }
    }
}


static const char *regExp_checkScriptProperty = "application object (\\S+) has (\\S+) ['\"](\\S+)['\"]";
static const int line_checkScriptProperty = __LINE__ + 1;
void CucumberApplicationManager::checkScriptProperty(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    checkPendingSender();

    if (args.size() != 3) {
        invokePlainSender(sender, "Expected 3 step arguments");
    }
    else {
        TasClientManager *clientManager = TasClientManager::instance();
        QString processId = mPidMap[mCurrentApplicationId];
        qDebug() << DPL << "looking for" << mCurrentApplicationId << "pid" << processId;
        TasClient *client = clientManager->findByProcessId(processId);

        if (!client) {
            QString errorMsg = QString("Application '%1' not found").arg(mCurrentApplicationId);
            doReplyOrRetry(&CucumberApplicationManager::checkScriptProperty, errorMsg, regExpPattern, args, sender);
        }
        else {
            QByteArray jsonData = QJson::Serializer().serialize(args);
            QHash<QString, QString> argMap;
            argMap["qjson"] = jsonData;
            argMap["regExp"] = regExpPattern;
            qDebug() << DPL << argMap;

            mRetryData.clear();
            mRetryTimer->stop();

            mPendingSender = sender;
            mPendingTimer->start();
            client->callFixture(this, "pluginResultCB", reinterpret_cast<quintptr>(mPendingSender),
                                "qtscriptfixture", CUCUMBER_STEP_DEFAULTACTION, argMap);
            // callFixture will call pluginResultCB, which will send response
        }
    }

}
#endif

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
        if (!mPidMap.contains(id)) {
            invokePlainSender(sender,
                              id.isEmpty() ? QString("No default application")
                                           : QString("No application with id '%1' found").arg(id));
        }
        else {
            // success, errorMsg is empty
            mCurrentApplicationId = id;
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

        if (mPidMap.contains(id)) {
            invokePlainSender(sender,
                              id.isEmpty() ? QString("Default application already exists")
                                           : QString("Application with id '%1' already exists").arg(id));
        }
        else {
            // not in pidMap, wait for registration
            TasClientManager *clientManager = TasClientManager::instance();
            QString processId = mPidMap.value(id);
            QString name = args.at(1).toString();
            TasClient *client = clientManager->findByApplicationName(name);
            if (client) {
                mPidMap.insert(id, client->processId());
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
    if (mPidMap.contains(id)) {
        // already started, wait for it to register
        TasClientManager *clientManager = TasClientManager::instance();
        QString processId = mPidMap.value(id);
        TasClient *client = clientManager->findByProcessId(processId);
        if (client) return QString(); // success
        else return QString("Application with process id " + processId + " did not register");
    }
    else {
        QString workingDirectory = mWorkingDirectoryPath.isEmpty() ? QDir::currentPath()
                                                                  : mWorkingDirectoryPath;
        QString responseData, responseErrorMessage;
        StartAppService::launchDetached(program, arguments, mStartEnvironment, workingDirectory, responseData, responseErrorMessage);
        qDebug() << DPL << "->" << program << arguments << '@' << workingDirectory  <<":" << responseData << responseErrorMessage;

        //bool result = QProcess::startDetached(program, arguments, workingDirectory, &pid);
        //qDebug() << DPL << "->" << program << arguments << '@' << workingDirectory  <<":" << result << pid;

        if (responseErrorMessage.isEmpty() && !responseData.isEmpty()) {
            mPidMap.insert(id, responseData);
            mCurrentApplicationId = id;
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
        //STEPINFO(callScriptMethod),
        //STEPINFO(checkScriptProperty),
        { 0, 0, 0 }
    };

    for (int ii=0; steps[ii].method && steps[ii].regExp && steps[ii].line; ++ii) {

        // registering method is of type:
        // void method(const QRegExp &regExp, QObject *object, const char *method, const char *sourceFile, int sourceLine);

        QString stepFileSpec = QFileInfo(__FILE__).canonicalFilePath();
        stepFileSpec = stepFileSpec.mid(stepFileSpec.indexOf("agent_qt/"))
                + ":" + QString::number(steps[ii].line);
        QMetaObject::invokeMethod(registrarObject, method,
                                  Q_ARG(QRegExp, QRegExp(steps[ii].regExp)),
                                  Q_ARG(QObject*, static_cast<QObject*>(this)),
                                  Q_ARG(const char*, steps[ii].method),
                                  Q_ARG(QString, stepFileSpec));
    }

    mPluginSteps.clear();
    mPluginSteps = CucumberUtils::readAllSteps();

    CucumberStepDataMap::const_iterator it;
    for (it = mPluginSteps.begin(); it != mPluginSteps.end(); ++it) {
        qDebug() << ".........." << it.value().toDebugString();
        QMetaObject::invokeMethod(registrarObject, method,
                                  Q_ARG(QRegExp, QRegExp(it.value().regExp)),
                                  Q_ARG(QObject*, static_cast<QObject*>(this)),
                                  Q_ARG(const char*, "callStepInPlugin"),
                                  Q_ARG(QString, it.value().stepFileSpec));

    }
}


void CucumberApplicationManager::beginScenario()
{
    mPidMap.clear();
}


void CucumberApplicationManager::endScenario()
{
    TasClientManager *clientManager = TasClientManager::instance();
    //    TasClient *client = clientManager->findByApplicationName(name);

    // avoid killing a process twice by converting to set of processId values
    QSet<QString> pidSet = QSet<QString>::fromList(mPidMap.values());
    for(QSet<QString>::iterator it = pidSet.begin(); it != pidSet.end(); ++it) {
        QString processId = *it;
        TasLogger::logger()->debug("CucumberApplicationManager::endScenario removing client pid " + processId);
        clientManager->removeClient(*it, true);
    }
    mPidMap.clear();
}


void CucumberApplicationManager::doRetryTimer()
{
    qDebug() << DPL << "entry";
    if (mRetryData.stepFn) {
        // call member function of this object through pointer retryData.stepFn
        ((this)->*(mRetryData.stepFn))(mRetryData.regExpPattern, mRetryData.args, mRetryData.sender.data());
    }
    else {
        qWarning() << DPL << "called with NULL stepFn, stopping timer and clearing retryData";
        mRetryTimer->stop();
        mRetryData.clear();
    }
}


void CucumberApplicationManager::doReplyOrRetry(InvokableStepFn fn, const QString &errorString,
                                                const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    qDebug() << DPL << "entry";
    if (mRetryData.hasCallback() && !mRetryData.equals(fn, regExpPattern, args, sender)) {
        qCritical() << DPL << "new updateRetries called when previous not finished!";
        invokePlainSender(mRetryData.sender.data(), "Cancelled, because of getting a new request!");
    }

    if (errorString.isEmpty()) {
        // success!
        invokePlainSender(sender, errorString);
    }
    else {
        if (mRetryData.stepFn == NULL) {
            // init retry data
            // first try was already done at this point
            mRetryData.retriesLeft = mRetryTimeout / mRetryTimer->interval();
            mRetryData.stepFn = fn;
            mRetryData.regExpPattern = regExpPattern;
            mRetryData.args = args;
            mRetryData.sender = sender;
            mRetryTimer->start();
        }
        else {
            // retryData was tested above, so is guaranteed to match method arguments
            // errorString was tested above, so is guaranteed to be non-empty

            // first try and minimum one retry done at his point
            --mRetryData.retriesLeft;
            if (mRetryData.retriesLeft > 0) {
                if (!mRetryTimer->isActive()) {
                    qWarning() << DPL << "retryTimer was not active on update, starting...";
                    mRetryTimer->start();
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
    mRetryData.clear();
    mRetryTimer->stop();

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
