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


#include <QCoreApplication>
#include <QList>
#include <QMutableListIterator>
#include <QMutableHashIterator>
#include <taslogger.h>
#include "tasnativeutils.h"

#include "tasclientmanager.h"
#include "tasserverservicemanager.h"



TasClientManager *TasClientManager::mInstance = 0;

/*!
    \class TasClientManager
    \brief TasClientManager managers the application which connect to the server.

    Application which connect to the server are stored to the pluginmanager.
    Applications which are started throuh the server also store the QPocess
    pointer which allows to control the process.

    TasClientManager is a singleton class so there is only one instance.

*/

TasClientManager::TasClientManager()
{
    mDataShare = new TasDataShare();
    mMessageCounter = 0;
}

/*!
  Destructor which clears the client list.
*/
TasClientManager::~TasClientManager()
{
    removeAllClients(false);
    delete mDataShare;
    mDataShare = 0;
}


/*!
  Returns the only instane of the TasClientManager.
 */
TasClientManager* TasClientManager::instance()
{
    if(mInstance == 0){
        mInstance = new TasClientManager();
    }
    return mInstance;
}


void TasClientManager::deleteInstance()
{
    if(mInstance){
        mInstance->deleteLater();
        mInstance = 0;
    }
}

/*!
  Add a new client with QProcess handle.
*/

TasClient* TasClientManager::addClient(quint64 processId, const QString& processName)
{
    QMutexLocker locker(&mMutex);
    TasLogger::logger()->info("TasClientManager::addClient " + processName);
    TasClient* app = 0;
    if(mClients.contains(processId)){
        app = mClients.value(processId);
        TasLogger::logger()->debug("TasClientManager::addClient existing client found by id!");
    }

    if(app == 0){
        app = new TasClient(processId);
        mClients.insert(processId, app);
    }

    if(!processName.isEmpty()){
        app->setApplicationName(processName);
    }
    return app;
}



/*!
  Adds a new client to the pluginmanager's internal list. If a client with the process id
  already exists only the data will be updated.
 */
TasClient* TasClientManager::addRegisteredClient(quint64 processId, const QString& processName, TasSocket* socket,
                                                 const QString& type)
{
    QMutexLocker locker(&mMutex);
    TasLogger::logger()->info("TasClientManager::addRegisteredClient " + processName);

    TasClient* app = 0;
    if(mClients.contains(processId)){
        app = mClients.value(processId);
        TasLogger::logger()->debug("TasClientManager::addRegisteredClient existing client found by id!");
    }

    if(app == 0){
        app = new TasClient(processId);
        mClients.insert(processId, app);
    }
    if(!processName.isEmpty()){
        app->setApplicationName(processName);
    }
    if(socket){
        app->setSocket(socket);
    }

    app->setPluginType(type);
    app->setRegistered();
    return app;
}

/*!
  Adds a new app to the list of started apps. If the app already exists it will be replaced.
 */
void TasClientManager::addStartedApp(const QString& processName, qint64 timestamp)
{
    QMutexLocker locker(&mMutex);
    //TasLogger::logger()->info("TasClientManager::addStartedApp " + processName + " " + QString::number(timestamp));
    mStartedApps[processName] = timestamp;
}

/*!
  Attempts to remove a client from the connected clients.
  The application process will be deleted which will cause a possible
  process to be killed if it is still running and the socket
  connection to close. The process state is checked however
  and if running it will not be deleted unless requested.

  Will wait for the process to finish for the time given.
  Returns false if app doed not close. Only applicable if process owned.
*/
void TasClientManager::removeClient(quint64 processId, bool kill)
{
    QMutexLocker locker(&mMutex);

    TasClient* app = removeByProcessId(processId);
    if(app){

        TasLogger::logger()->info("TasClientManager::removeClient " + app->applicationName());
        app->closeConnection();
        delete app;
        app = 0;
    }
    if(kill){
        TasNativeUtils::killProcess(processId);
    }
    TasLogger::logger()->info("TasClientManager::removeClient client count" + QString::number(mClients.size()));
}

void TasClientManager::removeMe(const TasClient &client)
{
    QMutexLocker locker(&mMutex);
    mClients.remove(client.pid());
    mStartedPids.removeAll(client.pid());
}


/*!
  Find the best match for the given model.
 */
TasClient* TasClientManager::findClient(TasCommandModel& request)
{
    //look for the client
    TasClient* client = 0;
    bool ok;
    quint64 startedPid = request.id().toULongLong(&ok);
    client = findByProcessId(startedPid);
    if(client == 0){
        client = findByApplicationName(request.name());
    }
    if(client == 0){
        client = latestClient();
        if(!verifyClient(client)){
            client = 0;
        }
    }
    return client;
}

/*!
  Searches for a client with the given process id. Returns null if no match
  found.
 */
TasClient* TasClientManager::findByProcessId(quint64 processId)
{
    if(processId == 0){
        return 0;
    }
    TasClient* match = 0;
    if(mClients.contains(processId)){
        if(mClients.value(processId)->socket()) {
            match = mClients.value(processId);
        }
    }
    if(!verifyClient(match)){
        match = 0;
    }
    return match;
}

/*!
  Searches for a client with the given application name. Returns null if no match
  found.
 */
TasClient* TasClientManager::findByApplicationName(const QString& applicationName)
{
    if(applicationName.isNull() || applicationName.isEmpty()){
        return 0;
    }
    TasClient* match = 0;
    int smallest = -1;
    foreach (TasClient* app, mClients){
        if(app->applicationName() == applicationName){
            //do not return socketless clients unless specifically requested
            if(app->socket()){
                if(smallest == -1){
                    smallest = app->upTime();
                    match = app;
                }
                else if(app->upTime() < smallest){
                    smallest = app->upTime();
                    match = app;
                }
            }
        }
    }
    if(!verifyClient(match)){
        match = 0;
    }
    return match;
}

/*!
  Returns the client which was added last (most recent).
  Returns null if the list is empty.
*/
TasClient* TasClientManager::latestClient()
{
    //    int pid = TasNativeUtils::pidOfActiveWindow(mClients.keys());
    int pid = TasNativeUtils::pidOfActiveWindow(mClients);
    if (pid != -1) {
        if(mClients.contains(pid)){
            TasClient* app = mClients.value(pid);
            if(app->socket()){
                return app;
            }
        }
    }
    //find latest
    TasClient* match = 0;
    int smallest = -1;
    if(!mClients.empty()){
        foreach (TasClient* app, mClients){
            if( app->socket() && app->pluginType() == TAS_PLUGIN){
                if(smallest == -1){
                    smallest = app->upTime();
                    match = app;
                }
                else if(app->upTime() < smallest){
                    smallest = app->upTime();
                    match = app;
                }
            }
        }
    }
    return match;
}


bool TasClientManager::verifyClient(TasClient* client)
{
    //ignore null checks
    if(!client) return true;

    bool valid = false;
    bool ok;
    quint64 pid = client->processId().toULongLong(&ok, 10);
    if(ok && pid != 0){
        if(!TasNativeUtils::verifyProcess(pid)){
            removeMe(*client);
            delete client;
            client = 0;
            valid = false;
            TasLogger::logger()->warning("TasClientManager::verifyClient process vanished had to remove client :" + QString::number(pid));
        } else {
            valid = true;
        }
    }
    return valid;
}

/*!
  Removes all clients from the list and kills all
  processes.
  After this call there will be no connected clients.
*/
void TasClientManager::removeAllClients(bool kill)
{
    QMutexLocker locker(&mMutex);
    foreach (TasClient* app, mClients){
        app->closeConnection();
        if(kill && mStartedPids.contains(app->pid())){
            TasNativeUtils::killProcess(app->pid());
        }
        mStartedPids.removeAll(app->pid());
        delete app;
    }
    mClients.clear();
}


void TasClientManager::addStartedPid(quint64 pid)
{
    mStartedPids.append(pid);
}

void TasClientManager::removeStartedPid(quint64 pid)
{
    mStartedPids.removeAll(pid);
}

/*!
  Searches for a client with the given process id and removes it from the list.
  Returns null if no match found.
 */
TasClient* TasClientManager::removeByProcessId(quint64 processId)
{
    if(processId == 0){
        return 0;
    }
    TasClient* app = 0;
    if(mClients.contains(processId)){
        app = mClients.take(processId);
        mStartedPids.removeAll(app->pid());
    }
    return app;
}

TasClient* TasClientManager::logMemClient()
{
    TasClient* match = 0;
    if(!mClients.empty()){
        foreach (TasClient* app, mClients){
            if(app->pluginType() == LOG_MEM_SRV){
                if(!app->socket()){
                    removeByProcessId(app->pid());
                    break;
                }
                match = app;
            }
        }
    }
    return match;
}

bool TasClientManager::writeStartupData(const QString& identifier, const TasSharedData& data)
{
    TasLogger::logger()->debug("TasClientManager::writeStartupData for identifier " + identifier);
    QString errMsg = "";
    bool retVal = mDataShare->storeSharedData(identifier, data, errMsg);
    if(!retVal){
        TasLogger::logger()->error("TasClientManager::writeStartupData error " + errMsg);
    }
    return retVal;
}

bool TasClientManager::detachFromStartupData(const QString& identifier)
{
    TasLogger::logger()->debug("TasClientManager::detachFromStartupData for identifier " + identifier);
    return mDataShare->detachSharedData(identifier);
}

/*!
  Return a data model with all application listed.
 */
void TasClientManager::applicationList(TasObject& parent)
{
    QMutexLocker locker(&mMutex);
    foreach (TasClient* app, mClients){
        if(app->socket()){
            TasObject& appObj = parent.addObject();
            appObj.setId(app->processId());
            appObj.setType(QString("application"));
            appObj.setName(app->applicationName());
        }
    }
}

/*!
  Return a data model with all started applications listed.
 */
void TasClientManager::startedApplicationsList(TasObject& parent)
{
    QMutexLocker locker(&mMutex);
    int i = 0;
    foreach (const QString& app, mStartedApps.keys()){
        if(app != NULL){
            TasObject& appObj = parent.addObject();
            appObj.setId(QString::number(i++));
            appObj.setType(QString("application"));
            appObj.setName(app);
            appObj.addAttribute("startTime", QDateTime::fromMSecsSinceEpoch(mStartedApps[app]).toString("yyyyMMddhhmmsszzz"));
        }
    }
}

/*!
    \class TasClient
    \brief TasAppliction utility class for application details

    Stores all of the details for plugins that connect to the server.
    QProcess is also stored in case the application is started from
    the server.

*/

TasClient::TasClient(quint64 processId)
{
    mSocket = 0;
    mCreationTime = QDateTime::currentMSecsSinceEpoch();
    mPid = processId;

}

TasClient::~TasClient()
{
    mSocket = 0;
}

/*!
  Returns the process id associated with the client.
 */
QString TasClient::processId()
{
    return QString::number(mPid);
}

const quint64& TasClient::pid() const
{
    return mPid;
}

/*!
  Returns the application name associated with the client.
 */
const QString& TasClient::applicationName()
{
    return mApplicationName;
}

/*!
  Set the application name associated with the client.
 */
void TasClient::setApplicationName(const QString& applicationName)
{
    mApplicationName = applicationName;
}

/*!
  Returns the socket connected to the client.
 */
TasSocket* TasClient::socket()
{
    return mSocket;
}

void TasClient::disconnected()
{
    TasLogger::logger()->debug(
        "TasClient::disconnected socket");

    if(mSocket){
        disconnect(mSocket, SIGNAL(socketClosed()), this, SLOT(disconnected()));
    }
    socketDied();
}

void TasClient::socketDied()
{
    if(mSocket){
        disconnect(mSocket, SIGNAL(destroyed()), this, SLOT(socketDied()));
    }
    //no process remove the client from the list
    //will register again if so wants
    mSocket = 0;
    TasClientManager::instance()->removeMe(*this);
    deleteLater();
}

void TasClient::closeConnection()
{
    if(mSocket){
        disconnect(mSocket, SIGNAL(socketClosed()), this, SLOT(disconnected()));
        mSocket->closeConnection();
    }
}

/*!
  Set the socket connected to the client.
 */
void TasClient::setSocket(TasSocket* socket)
{
    mSocket = socket;
    connect(mSocket, SIGNAL(socketClosed()), this, SLOT(disconnected()));
    connect(mSocket, SIGNAL(destroyed()), this, SLOT(socketDied()));
}

int TasClient::upTime()
{
    return QDateTime::currentMSecsSinceEpoch() - mCreationTime;
}


bool TasClient::operator ==(const TasClient &client) const
{
    return pid() == client.pid();
}

void TasClient::setRegistered()
{
    emit registered(mPid);
}

/*!
  In symbian application are most often identified by uid instead of
  process ids. Store symbian uid to make it possible to look for the
  correct app with it.
 */
void TasClient::setApplicationUid(const QString& uid)
{
    mApplicationUid = uid;
}

/*!
  Return the application uid
*/
QString TasClient::applicationUid()
{
    return mApplicationUid;
}


void TasClient::setPluginType(const QString& pluginType)
{
    mPluginType = pluginType;
}

QString TasClient::pluginType()
{
    return mPluginType;
}

void TasClient::callFixture(QObject *sender, const char *resultMethod, quintptr callId,
                            const QString &pluginName, const QString &actionName, QHash<QString, QString> parameters)
{
    if (pluginName.isEmpty() || actionName.isEmpty()) return;

    TasCommandModel* model = TasCommandModel::createModel();

    model->addDomAttribute("service", FIXTURE);

    TasTarget& target = model->addTarget();
    target.addDomAttribute("TasId", "app"); // this value isn't supposed to be used anywhere
    target.addDomAttribute("type", TYPE_APPLICATION_VIEW);

    TasCommand& command = target.addCommand();
    command.addDomAttribute("name", "Fixture");
    command.addDomAttribute("plugin", pluginName);
    command.addDomAttribute("method", actionName);
    foreach(const QString &key, parameters.keys()) {
        command.addApiParameter(key, parameters.value(key), "QString");
    }

    QByteArray replyData;
    bool success;
    TasMessage reply;


    if (socket()->syncRequest(callId, model->sourceString(), reply)) {
        replyData = reply.data();
        success = !reply.isError();
    }
    else {
        replyData = "Fixture request could not be sent.";
        success = false;

    }

    if (sender && resultMethod && *resultMethod) {
        QMetaObject::invokeMethod(sender, resultMethod, Qt::QueuedConnection,
                                  Q_ARG(bool, success),
                                  Q_ARG(QString, QString(replyData)),
                                  Q_ARG(quintptr, callId));
    }

    delete model;
}
