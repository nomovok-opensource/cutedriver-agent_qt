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

#include "cucumberwireprotocolserver.h"

#include "cucumberapplicationmanager.h"

#include <taslogger.h>

#include <testabilitysettings.h>

#include <QTcpServer>
#include <QTcpSocket>

#include <QDebug>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>

#include <qjson/parser.h>
#include <qjson/serializer.h>


//#define DP "CucumberWireprotocolServer" << __FUNCTION__
//#define DPL "CucumberWireprotocolServer" << __FUNCTION__ << __LINE__


class CucumberWireprotocolServer_StepData
{
public:
    QRegExp regExp;
    QWeakPointer<QObject> targetObject;
    const char *targetMethod;
    QVariant source;

    CucumberWireprotocolServer_StepData() : targetMethod(NULL) {}

    CucumberWireprotocolServer_StepData(const CucumberWireprotocolServer_StepData &other) :
        regExp(other.regExp)
      , targetObject(other.targetObject)
      , targetMethod(other.targetMethod)
      , source(other.source)
    {}

    CucumberWireprotocolServer_StepData(const QRegExp &regExp, QObject *targetObject, const char *targetMethod, QVariant source=QVariant()) :
        regExp(regExp)
      , targetObject(targetObject)
      , targetMethod(targetMethod)
      , source(source)
    {}

    CucumberWireprotocolServer_StepData(const QRegExp &regExp, const QWeakPointer<QObject> targetObject, const char *targetMethod, QVariant source=QVariant()) :
        regExp(regExp)
      , targetObject(targetObject)
      , targetMethod(targetMethod)
      , source(source)
    {}

    bool isValid() { return (!targetObject.isNull() && targetMethod != NULL); }
    void invalidate() { targetObject.clear(); targetMethod = NULL; }

    bool hasSource() { return !source.isNull(); }
};


#if 0
static const int LINE_invokeDebugDump = __LINE__ + 1;
void CucumberWireprotocolServer::invokeDebugDump(const QString &regExpPattern, const QVariantList &args, QObject *sender)
{
    //qDebug() << DP << regExpPattern << args;
    if (sender) {
        QMetaObject::invokeMethod(sender, "cucumberSuccessResponse", Qt::QueuedConnection);
    }
}
#endif


CucumberWireprotocolServer::CucumberWireprotocolServer(quint16 port, QObject *parent) :
    QObject(parent)
  , mServer(new QTcpServer(this))
  , mListenPort(port)
  , mPendingResponse(false)
  , mResponseTimeoutTimer(new QTimer(this))
  , mAppManager(new CucumberApplicationManager(this))
{
    mResponseTimeoutTimer->setSingleShot(true);
    connect(mResponseTimeoutTimer, SIGNAL(timeout()), SLOT(responseTimeout()));

    connect(mServer, SIGNAL(newConnection()), SLOT(handleNewConnect()));
    mServer->setMaxPendingConnections(1);
    connect(this, SIGNAL(gotJSONMessage(QVariant,QIODevice*)), SLOT(handleJSONMessage(QVariant,QIODevice*)));


#if 0
    CucumberWireprotocolServer_StepData tmpStep;
    tmpStep.targetObject = this;
    tmpStep.targetMethod = "invokeDebugDump";

    // register dummy step
    QStringList stepDefs(QStringList()
                         << "I perform pairs:"
                         << "current application is\\s+(.+)");

    foreach(QString pattern, stepDefs) {
        registerStep(QRegExp(pattern), this, "invokeDebugDump", QString("%1:%2").arg(__FILE__).arg(LINE_invokeDebugDump));
    }
#endif

    //reRegisterSteps();
}


CucumberWireprotocolServer::~CucumberWireprotocolServer()
{
    while (!mSteps.isEmpty()) {
        CucumberWireprotocolServer_StepData *step = mSteps.takeLast();
        if (step) delete step;
    }
}


/*static*/
bool CucumberWireprotocolServer::enabledInSettings()
{
    QVariant value = TestabilitySettings::settings()->getValue(CUCUMBER_WIRE_SERVER);
    if(value.isValid() && value.canConvert<QString>()){
        if(value.toString() == "on"){
            return true;
        }
    }
    return false;
}


QString CucumberWireprotocolServer::addressString()
{
    return mServer->serverAddress().toString() + ":" + QString::number(mServer->serverPort());
}


void CucumberWireprotocolServer::registerStep(const QRegExp &regExp, QObject *object,
                                              const char *method, const QString &sourceFileSpec)
{
    //qDebug() << DPL << regExp.pattern() << method << sourceFileSpec;
    mSteps << new CucumberWireprotocolServer_StepData(regExp, object, method, sourceFileSpec);
}


void CucumberWireprotocolServer::closeAllClients()
{
    QList<QIODevice*> ioChildren = findChildren<QIODevice*>();
    foreach(QIODevice *child, ioChildren) {
        child->close();
    }
}


void CucumberWireprotocolServer::close()
{
    closeAllClients();
    mServer->close();
}


int CucumberWireprotocolServer::start()
{
    if (mServer->isListening()) {
        if (mServer->serverPort() == mListenPort) {
            return mListenPort;
        }
        else {
            mServer->close();
        }
    }

    if (!mServer->listen(QHostAddress::Any, mListenPort)) {
        mLastErrorString = mServer->errorString();
        TasLogger::logger()->error(QString("CucumberWireprotocolServer::start listen error %1 on port %2").arg(mLastErrorString).arg(mListenPort));
        return -1;
    }

    TasLogger::logger()->info("CucumberWireprotocolServer::start listening on port " + QString::number(mListenPort));

    return mServer->serverPort();
}



void CucumberWireprotocolServer::reRegisterSteps()
{
    while (!mSteps.isEmpty()) {
        CucumberWireprotocolServer_StepData *step = mSteps.takeLast();
        if (step) delete step;
    }
    mSteps.clear();
    // ask CucumberApplicationManager instance to register it's steps
    mAppManager->registerSteps(this, "registerStep");
}



void CucumberWireprotocolServer::handleNewConnect()
{
    QTcpSocket *connection = mServer->nextPendingConnection();
    if (connection) {
        reRegisterSteps();

        connect(connection, SIGNAL(disconnected()), SLOT(connectionDisconnect()));
        connect(connection, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(connectionError()));
        connect(connection, SIGNAL(readyRead()), SLOT(connectionReadyRead()));
    }
}


void CucumberWireprotocolServer::connectionDisconnect()
{
    TasLogger::logger()->info("CucumberWireprotocolServer::connectionDisconnect");

    QIODevice *connection = qobject_cast<QIODevice*>(sender());
    if (connection) {
        mReadBufferMap.remove(connection);
        connection->deleteLater();
    }
}


void CucumberWireprotocolServer::connectionError()
{

    QIODevice *connection = qobject_cast<QIODevice*>(sender());
    if (connection) {
        TasLogger::logger()->info("CucumberWireprotocolServer::connectionError " + connection->errorString());

        mReadBufferMap.remove(connection);
        connection->deleteLater();
    }
}


// Warning: code below assumes an entire single JSON message is read with readAll
// if parsing fails, it just assumes not all data was recieves, and waits for more
// and then retries parsing.
void CucumberWireprotocolServer::connectionReadyRead()
{
    QIODevice *connection = qobject_cast<QIODevice*>(sender());
    if (connection) {
        QByteArray data = connection->readAll();

        data.prepend(mReadBufferMap[connection]);

        if (data.trimmed().isEmpty()) {
            //qDebug() << DPL << "ignoring empty data";
        }
        else {
            QJson::Parser parser;
            bool ok;
            QVariant result = parser.parse(data, &ok);

            if (ok) {
                emit gotJSONMessage(result, connection);
                data.clear();
            }
            else {
                //qDebug() << DPL << "parse error, wait for more data";
            }
        }
        mReadBufferMap[connection] = data;
    }
    else {
        //qDebug() << DP << "ignoring signal from bad sender class:" << sender()->metaObject()->className();
    }
}



void CucumberWireprotocolServer::handleJSONMessage(QVariant message, QIODevice *connection)
{
    Q_ASSERT(connection);

    if (mPendingResponse) {
        //Q_ASSERT(mResponseTimeoutTimer->isActive());
        TasLogger::logger()->info("CucumberWireprotocolServer::handleJSONMessage got new request before finishing previous");
        connection->close();
        return;
    }

    QVariantList msgList = message.toList();

    QString errorMessage;
    QVariantList outMsg;

    if (message.type() != QVariant::List) {
        //qDebug() << DP << "closing connection: badly structured data:" << message.typeName() << "when List expected";
        errorMessage = "incompatible message structure (expected a list structure)";
    }
    else if (msgList.isEmpty() || msgList.size() > 2) {
        //qDebug() << DP << "closing connection: badly structured data: list size" << msgList.size() << "not in range 1-2";
        errorMessage = "incompatible message structure (expected 1 or 2 list items)";
    }
    else if (!msgList.at(0).type() == QVariant::String) {
        //qDebug() << DP << "closing connection: badly structured data: 1st list item type not string:" << msgList.at(0).typeName();
        errorMessage = "incompatible message structure (expected string as the 1st list item)";
    }
    else if (msgList.size() == 2 && msgList.at(1).type() != QVariant::Map) {
        //qDebug() << DP << "closing connection: badly structured data: 2nd list item type not map:" << msgList.at(1).typeName();
        errorMessage = "incompatible message structure (expected map as the 2nd list item)";
    }
    else {
        QString msgName = msgList.first().toString();
        QVariantMap msgMap(msgList.value(1, QVariantMap()).toMap());

        if (msgName == "step_matches") {
            QString name = msgMap.value("name_to_match").toString();

            QVariantList outList = findMatchingSteps(name);
            outMsg << "success" << QVariant(outList);
        }

        else if (msgName == "begin_scenario") {
            mAppManager->beginScenario();
            outMsg << "success";
        }

        else if (msgName == "invoke") {
            if (!startStep(msgMap, connection)) {
                outMsg << "pending" << "step not done";
            }
            // else pending response data set
        }

        else if (msgName == "end_scenario") {
            mAppManager->endScenario();
            outMsg << "success";
        }

        else if (msgName == "snippet_text") {
            outMsg << "success";
        }

        else {
            errorMessage = "unknown operation: " + msgName;
            TasLogger::logger()->warning("CucumberWireprotocolServer::handleJSONMessage closing coneection, because received unknown request: " + msgName);
            connection->close();
            return;
        }
    }

    if (!mPendingResponse) {
        if (outMsg.isEmpty() && errorMessage.isEmpty()) {
            errorMessage = "nothingness of internal logic error";
        }

        if (!errorMessage.isEmpty()) {
            outMsg.clear();
            QVariantMap failMap;
            failMap.insert("message", errorMessage);
            outMsg << "fail" << failMap;
        }
        writeResponse(outMsg, connection);
    }
    else {
        //qDebug() << DPL << "RESPONSE PENDING...";
    }
}


QVariantList CucumberWireprotocolServer::findMatchingSteps(const QString &name)
{
    QVariantList retList;
    for (int ind = 0; ind < mSteps.size(); ++ind) {
        CucumberWireprotocolServer_StepData *step = mSteps.at(ind);
        if (step && step->isValid() && step->regExp.exactMatch(name)) {
            QVariantList argsList;
            for(int ii=1; ii <= step->regExp.captureCount(); ++ii) {
                QVariantMap argInfoMap;
                argInfoMap.insert("val", step->regExp.cap(ii));
                argInfoMap.insert("pos", step->regExp.pos(ii)); // must be number
                argsList << argInfoMap;
            }
            QVariantMap idMap;
            idMap.insert("id", QByteArray::number(ind+1));
            idMap.insert("args", argsList);
            if (step->hasSource()) idMap.insert("source", step->source);
            idMap.insert("regexp", step->regExp.pattern());
            retList.append(idMap);
        }
    }

    return retList;
}

bool CucumberWireprotocolServer::startStep(const QVariantMap &args, QIODevice *connection)
{
    int ind = args.value("id").toInt() - 1;
    CucumberWireprotocolServer_StepData *step = mSteps.value(ind, NULL);
    if (step && step->isValid()) {
        QVariantList argsList = args.value("args", QVariantList()).toList();
        QMetaObject::invokeMethod(
                    step->targetObject.data(), step->targetMethod, Qt::QueuedConnection,
                    Q_ARG(QString, step->regExp.pattern()), Q_ARG(QVariantList, argsList),
                    Q_ARG(QObject*, this));
        setResponseTimeout(10, connection); // sets mPendingResponse, which is tested below
        return true;
    }
    else {
        return false;
    }
}


void CucumberWireprotocolServer::responseTimeout()
{
    if (!mPendingResponse) {
        //qDebug() << DP << "no pending response, ignoring";
        resetResponseTimeout();
    }
    else {
        cucumberFailResponse(QString("Timeout waiting for response."));
    }
}


void CucumberWireprotocolServer::cucumberFailResponse(const QString &errorString)
{
    QVariantList outMsg;
    QVariantMap failMap;
    failMap.insert("message", errorString);
    outMsg << "fail" << failMap;
    cucumberResponse(outMsg);
}


void CucumberWireprotocolServer::cucumberSuccessResponse()
{
    QVariantList outMsg;
    outMsg << "success";
    cucumberResponse(outMsg);
}


void CucumberWireprotocolServer::cucumberResponse(const QVariantList &outMsg)
{
    if (!mPendingResponse) {
        TasLogger::logger()->debug("CucumberWireprotocolServer::cucumberResponse called without pending response");
        return;
    }

    QIODevice *connection = mPendingConnection.data();
    resetResponseTimeout();

    if (!connection) {
        TasLogger::logger()->debug("CucumberWireprotocolServer::cucumberResponse for expired connection");
    }
    else {
        TasLogger::logger()->debug("CucumberWireprotocolServer::cucumberResponse sending response");
        writeResponse(outMsg, connection);
    }
}


void CucumberWireprotocolServer::setResponseTimeout(int sec, QIODevice *connection)
{
    mPendingConnection = connection;
    mPendingResponse = true;
    mResponseTimeoutTimer->start(sec*1000);
}


void CucumberWireprotocolServer::resetResponseTimeout()
{
    mPendingConnection.clear();
    mPendingResponse = false;
    mResponseTimeoutTimer->stop();
}


void CucumberWireprotocolServer::writeResponse(const QVariantList &outMsg, QIODevice *connection)
{
    QJson::Serializer serializer;
    //serializer.setIndentMode(QJson::IndentFull);
    QByteArray outBuf = serializer.serialize(outMsg);
    if (!outBuf.endsWith('\n')) outBuf.append('\n');
    qint64 written = connection->write(outBuf);
    Q_ASSERT(written == outBuf.size());

}

