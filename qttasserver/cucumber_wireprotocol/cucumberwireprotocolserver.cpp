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
#include "cucumberstepdata.h"

#include <QTcpServer>
#include <QTcpSocket>

#include <QDebug>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <qjson/src/parser.h>
#include <qjson/src/serializer.h>

#define DP "CucumberWireprotocolServer" << __FUNCTION__
#define DPL "CucumberWireprotocolServer" << __FUNCTION__ << __LINE__


static const int LINE_invokeDebugDump = __LINE__ + 1;
QString CucumberWireprotocolServer::invokeDebugDump(const QString &regExpPattern, const QVariantList &args)
{
    qDebug() << DP << regExpPattern << args;
    return QString();
}



CucumberWireprotocolServer::CucumberWireprotocolServer(quint16 port, QObject *parent) :
    QObject(parent)
  , mServer(new QTcpServer(this))
  , mListenPort(port)
  , appManager(new CucumberApplicationManager(this))
{
    connect(mServer, SIGNAL(newConnection()), SLOT(handleNewConnect()));
    mServer->setMaxPendingConnections(1);
    connect(this, SIGNAL(gotJSONMessage(QVariant,QIODevice*)), SLOT(handleJSONMessage(QVariant,QIODevice*)));

    CucumberStepData tmpStep;
    tmpStep.targetType = CucumberStepData::ServerInternal;
    tmpStep.targetObject = this;
    tmpStep.targetMethod = "invokeDebugDump";

    QStringList stepDefs(QStringList()
                         << "I perform pairs:"
                         << "current application is\\s+(.+)");

    foreach(QString pattern, stepDefs) {
        registerStep(QRegExp(pattern), this, "invokeDebugDump", __FILE__, LINE_invokeDebugDump);
    }

    appManager->registerSteps(this, "registerStep");
}


CucumberWireprotocolServer::~CucumberWireprotocolServer()
{
    while (!steps.isEmpty()) {
        CucumberStepData *step = steps.takeLast();
        if (step) delete step;
    }
}

QString CucumberWireprotocolServer::addressString()
{
    return mServer->serverAddress().toString() + ":" + QString::number(mServer->serverPort());
}


void CucumberWireprotocolServer::registerStep(const QRegExp &regExp,
                                                      QObject *object, const char *method,
                                                      const char *sourceFile, int sourceLine)
{
    qDebug() << DPL << regExp.pattern() << method << sourceFile << sourceLine;
    steps << new CucumberStepData(regExp, CucumberStepData::ServerInternal, object, method,
                                  QString("%1:%2:%3").arg(sourceFile).arg(sourceLine).arg(method));
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
            qDebug() << DP << "already listeining on" << addressString();
            return mListenPort;
        }
        else {
            qDebug() << DP << "stopping to listen on" << addressString();
            mServer->close();
        }
    }

    if (!mServer->listen(QHostAddress::Any, mListenPort)) {
        mLastErrorString = mServer->errorString();
        qDebug() << DP << mListenPort << "listen error:" << mLastErrorString;
        return -1;
    }

    qDebug() << DP << "started to listen on" << addressString();

    return mServer->serverPort();
}


void cucumberStep(const QString &stepText, const QRegExp &matchedRegEx)
{
    qDebug() << DP << stepText << "with" << matchedRegEx.captureCount() << "matches:" << matchedRegEx.capturedTexts();
}


void CucumberWireprotocolServer::handleNewConnect()
{
    QTcpSocket *connection = mServer->nextPendingConnection();
    if (!connection) {
        qDebug() << DP << "got signal for new connection, but none were found";
        return;
    }

    connect(connection, SIGNAL(disconnected()), SLOT(connectionDisconnect()));
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(connectionError()));
    connect(connection, SIGNAL(readyRead()), SLOT(connectionReadyRead()));
    connect(connection, SIGNAL(bytesWritten(qint64)), SLOT(connectionBytesWritten(qint64)));
}


void CucumberWireprotocolServer::connectionDisconnect()
{
    qDebug() << DP;

    QIODevice *connection = qobject_cast<QIODevice*>(sender());
    if (connection) mReadBufferMap.remove(connection);

}


void CucumberWireprotocolServer::connectionError()
{
    qDebug() << DP;

    QIODevice *connection = qobject_cast<QIODevice*>(sender());
    if (connection) mReadBufferMap.remove(connection);
}


static void makeDumpableData(QByteArray &data)
{
    for (int ii=0; ii < data.size(); ++ii) {
        char byte = data[ii];
        if (byte < 32) data[ii] = '§';
        else if (byte > 126) data[ii] = '§';
    }
}



// Warning: code below assumes an entire single JSON message is read with readAll
// if parsing fails, it just assumes not all data was recieves, and waits for more
// and then retries parsing.
void CucumberWireprotocolServer::connectionReadyRead()
{
    QIODevice *connection = qobject_cast<QIODevice*>(sender());
    if (connection) {
        QByteArray data = connection->readAll().trimmed();

        QByteArray copydata(data);
        makeDumpableData(copydata);
        qDebug() << DPL << "new data:" << copydata.size() << copydata;

        data.prepend(mReadBufferMap[connection]);

        if (data.isEmpty()) {
            qDebug() << DPL << "ignoring empty data";
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
                qDebug() << DPL << "parse error, wait for more data";
            }
        }
        mReadBufferMap[connection] = data;
    }
    else {
        qDebug() << DP << "ignoring signal from bad sender class:" << sender()->metaObject()->className();
    }
}


void CucumberWireprotocolServer::connectionBytesWritten(qint64 bytes)
{
    qDebug() << DP << bytes;
}

void CucumberWireprotocolServer::handleJSONMessage(QVariant message, QIODevice *connection)
{
    QVariantList msgList = message.toList();

    //qDebug() << DPL << result.typeName() << resultList.size() << ':';
    //qDebug() << DPL << result;
    qDebug() << DPL << msgList.value(0).toString();

    QString errorMessage;
    QVariantList outData;

    if (message.type() != QVariant::List) {
        qDebug() << DP << "closing connection: badly structured data:" << message.typeName() << "when List expected";
        errorMessage = "incompatible message structure (expected a list structure)";
    }
    else if (msgList.isEmpty() || msgList.size() > 2) {
        qDebug() << DP << "closing connection: badly structured data: list size" << msgList.size() << "not in range 1-2";
        errorMessage = "incompatible message structure (expected 1 or 2 list items)";
    }
    else if (!msgList.at(0).type() == QVariant::String) {
        qDebug() << DP << "closing connection: badly structured data: 1st list item type not string:" << msgList.at(0).typeName();
        errorMessage = "incompatible message structure (expected string as the 1st list item)";
    }
    else if (msgList.size() == 2 && msgList.at(1).type() != QVariant::Map) {
        qDebug() << DP << "closing connection: badly structured data: 2nd list item type not map:" << msgList.at(1).typeName();
        errorMessage = "incompatible message structure (expected map as the 2nd list item)";
    }
    else {
        QString msgName = msgList.first().toString();
        QVariantMap msgMap(msgList.value(1, QVariantMap()).toMap());

        if (msgName == "step_matches") {
            QString name = msgMap.value("name_to_match").toString();

            outData << "success";
            QVariantList outList;

            for (int ind = 0; ind < steps.size(); ++ind) {
                CucumberStepData *step = steps.at(ind);
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
                    outList.append(idMap);
                }
            }
            outData << QVariant(outList);
        }
        else if (msgName == "begin_scenario") {
            outData << "success";
        }
        else if (msgName == "invoke") {
            int ind = msgMap.value("id").toInt() - 1;
            CucumberStepData *step = steps.value(ind, NULL);
            if (step && step->isValid()) {
                QString ret;
                QVariantList argsList = msgMap.value("args", QVariantList()).toList();
                QMetaObject::invokeMethod(
                            step->targetObject.data(), step->targetMethod, Qt::DirectConnection,
                            Q_RETURN_ARG(QString, ret),
                            Q_ARG(QString, step->regExp.pattern()), Q_ARG(QVariantList, argsList));
                if (ret.isEmpty()) {
                    outData << "success";
                }
                else {
                    errorMessage = ret;
                }
            }
            else {
                outData << "pending" << "step not done";
            }
        }
        else if (msgName == "end_scenario") {
            outData << "success";
        }
        else if (msgName == "snippet_text") {
            outData << "success";
        }
        else {
            errorMessage = "unknown operation: " + msgName;
            qDebug() << DP << "closing connection: unknown request:" << msgName;

            connection->close();
        }
    }

    if (outData.isEmpty() && errorMessage.isEmpty()) {
        errorMessage = "nothingness of internal logic error";
    }

    if (!errorMessage.isEmpty()) {
        outData.clear();
        QVariantMap failMap;
        failMap.insert("message", errorMessage);
        outData << "fail" << failMap;
    }

    QJson::Serializer serializer;
    //serializer.setIndentMode(QJson::IndentFull);
    QByteArray outBuf = serializer.serialize(outData);
    if (!outBuf.endsWith('\n')) outBuf.append('\n');
    qDebug() << DPL << outBuf.size() << outBuf.trimmed();
    qint64 written = connection->write(outBuf);
    //qDebug() << DPL << "wrote" << written << '/' << outBuf.size() << "bytes";
    Q_ASSERT(written == outBuf.size());
}


