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
    removeAllClients();
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

TasClient* TasClientManager::addClient(const QString& processId, const QString& processName, QProcess *process)
{
    QMutexLocker locker(&mMutex);
    TasLogger::logger()->info("TasClientManager::addClient " + processName);
    TasClient* app = 0;
    if(mClients.contains(processId)){
        app = mClients.value(processId);
        TasLogger::logger()->debug("TasClientManager::addClient existing client found by id!");
    }

#ifdef Q_OS_SYMBIAN 
    if(app == 0 && !processName.isEmpty()){
        app = findByApplicationName(processName, true);
        if(app){
            TasLogger::logger()->debug("TasClientManager::addClient existing client found by name!");
        }
    }
#endif   

    if(app == 0){
        app = new TasClient(processId);
        mClients.insert(processId, app);
    }

    if(!processName.isEmpty()){
        app->setApplicationName(processName);
    }

    if(process){
        app->setProcess(process);
    }
    return app;
}

/*!
  Adds a new client to the pluginmanager's internal list. If a client with the process id
  already exists only the data will be updated.
 */
TasClient* TasClientManager::addRegisteredClient(const QString& processId, const QString& processName, TasSocket* socket, 
                                                 const QString& type, QString applicationUid)
{
    QMutexLocker locker(&mMutex);   
    TasLogger::logger()->info("TasClientManager::addRegisteredClient " + processName);
    //check for processes that are not running and can be removed
    checkJammedProcesses();
    //check clients that were never reqistered
    removeGhostClients();

    TasClient* app = 0;
    if(mClients.contains(processId)){
        app = mClients.value(processId);
        TasLogger::logger()->debug("TasClientManager::addRegisteredClient existing client found by id!");
    }

#ifdef Q_OS_SYMBIAN 
    if(app == 0 && !processName.isEmpty()){
        app = findByApplicationName(processName, true);
        if(app && app->applicationUid() == applicationUid){
            TasLogger::logger()->debug("TasClientManager::addRegisteredClient existing client found by name!");
        }
        else{
            app = 0;
        }
    }
#endif   

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

#ifdef Q_OS_SYMBIAN 
    app->setApplicationUid(applicationUid);
#endif

    app->setPluginType(type);
    app->setRegistered();
    return app;
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
TasClientManager::CloseStatus TasClientManager::removeClient(const QString& processId, bool kill, int time)
{
    QMutexLocker locker(&mMutex);   
    TasClientManager::CloseStatus status = TasClientManager::Ok;
    TasClient* app = removeByProcessId(processId);
    if(app){
        app->stopProcessMonitor();
        TasLogger::logger()->info("TasClientManager::removeClient " + app->applicationName());
        app->closeConnection();
        if( app->process() && app->process()->state() == QProcess::Running ){

            bool closed = app->process()->waitForFinished(time);
        
            if(!closed){                
                status = TasClientManager::Stalled;
                if(kill){
                    TasLogger::logger()->error("TasClientManager::removeClient app did not close properly killing it");
                    app->killProcess();
                }
                else{
                    //check for possibly jammed processes before delete
                    //taclient will not delete running processes
                    mJammedProcesses.insert(processId, app->process());
                    TasLogger::logger()->error("TasClientManager::removeClient app did not close properly added jammed list");
                }
            }
            else if( app->process() && app->process()->exitStatus() == QProcess::CrashExit){
                TasLogger::logger()->error("TasClientManager::removeClient app crashed in exit.");
                status = TasClientManager::Crashed;
            }
        }
        delete app;
        app = 0;
    }
    else if(kill && mJammedProcesses.contains(processId)){
        QProcess* process = mJammedProcesses.value(processId);
        process->kill();
        delete process;
        process = 0;
        mJammedProcesses.remove(processId);
    }
    return status;
}

void TasClientManager::removeMe(const TasClient &client)
{
    QMutexLocker locker(&mMutex);   
    mClients.remove(client.processId());
}


/*!
  Find the best match for the given model.
 */
TasClient* TasClientManager::findClient(TasCommandModel& request)
{    
    //look for the client
    TasClient* client = 0;   
    client = findByProcessId(request.id());
#ifdef Q_OS_SYMBIAN   
    if(client == 0){
        client = findByApplicationUid(request.uid());
    }
#endif
    if(client == 0){
        client = findByApplicationName(request.name());
    }
    if(client == 0){
        client = latestClient();
    }   

    return client;
}

TasClientManager::ClientError TasClientManager::moveClientToCrashedList(const QString& processId)
{
    QMutexLocker locker(&mMutex);
    
    QString count;
    count.setNum(mCrashedProcesses.count());
    TasLogger::logger()->debug( "TasClientManager::moveClientToCrashedList: crashed clients: " + count );
    
    TasClientManager::ClientError error = TasClientManager::NoError;
    TasClient* client = removeByProcessId(processId);
    if (client) {
    	mCrashedProcesses.insertMulti(processId, client);
        TasLogger::logger()->debug(
    			"TasClientManager::moveClientToCrashedList: inserted client "
    			+ client->applicationName() + ", pid " + processId);
    } else {
    	error = TasClientManager::RemoveError;
    	TasLogger::logger()->debug(
    			"TasClientManager::moveClientToCrashedList: removing client "
    			+ client->applicationName() + ", pid " + processId + " failed");
    }

    count.setNum(mCrashedProcesses.count());
    TasLogger::logger()->debug("TasClientManager::moveClientToCrashedList: crashed clients: " + count);
            
    return error;
}

void TasClientManager::crashedApplicationList(TasObject& parent)
{
    QMutexLocker locker(&mMutex);
    TasLogger::logger()->debug("TasClientManager::crashedApplicationList");
    		
    foreach (TasClient* crashedClient, mCrashedProcesses) {
		TasObject& clientObj = parent.addObject();
		clientObj.setId(crashedClient->processId());
		clientObj.setType(QString("application"));
		clientObj.setName(crashedClient->applicationName());
		clientObj.addAttribute(QString("crashTime"), crashedClient->crashTime().toString());
		TasLogger::logger()->debug(
				"  " + crashedClient->applicationName() 
				+ ", pid " + crashedClient->processId());
    }
}

void TasClientManager::emptyCrashedApplicationList()
{
	TasLogger::logger()->debug("TasClientManager::emptyCrashedApplicationList");
	
	foreach (TasClient* crashedClient, mCrashedProcesses) {
		delete crashedClient;
		crashedClient = 0;
	}
	mCrashedProcesses.clear();
	
	QString count;
	count.setNum(mCrashedProcesses.count());
	TasLogger::logger()->debug( "TasClientManager::emptyCrashedApplicationList: crashed clients: " + count );
}

/*!
  Checks for processes that were not removed when the client was removed.
  Running processes are not deleted if the client is removed and in some cases the
  process may still be running when the app is unregistered. 
 */
void TasClientManager::checkJammedProcesses()
{
    QMutableHashIterator<QString, QProcess*> iter(mJammedProcesses);
    while (iter.hasNext()) {
        iter.next();
        QProcess* process = iter.value();
        if(process && process->state() != QProcess::Running){
            delete process;
            process = 0;
            mJammedProcesses.remove(iter.key());
        }
    }
}

void TasClientManager::removeGhostClients()
{
    QMutableHashIterator<QString, TasClient*> iter(mClients);
    while (iter.hasNext()) {
        iter.next();
        TasClient* app = iter.value();
        if(app == 0){
            mClients.remove(iter.key());
        }
        else if(!app->socket()){
            if(app->upTime() > 600000){
                TasLogger::logger()->debug("TasClientManager::removeGhostClients removing client " + app->processId() +  " " 
                                           + app->applicationName());

                detachFromStartupData(app->applicationName());
                app->killProcess();
                mClients.remove(iter.key());
                delete app;
                app = 0;
            }
            else if(app->processId().isEmpty()){
                mClients.remove(iter.key());
                delete app;
                app = 0;
            }
        }
    }
}


/*!
  Removes all clients from the list and kills all
  processes.
  After this call there will be no connected clients.
*/
void TasClientManager::removeAllClients()
{
    QMutexLocker locker(&mMutex);   
    foreach (TasClient* app, mClients){
        app->closeConnection();
        QProcess* process = app->process();
        if(process && process->state() == QProcess::Running){
            process->kill();
        }
        else{
            bool ok;
            quint64 pid = app->processId().toULongLong(&ok, 10);       
            if(ok && pid != 0){
                TasNativeUtils::killProcess(pid);
            }

        }
        delete app;
        app = 0;
    }
    mClients.clear();

    foreach (QProcess* process, mJammedProcesses){
        delete process;
        process = 0;
    }
    mJammedProcesses.clear();
    
    foreach (TasClient* crashedClient, mCrashedProcesses) {
    	delete crashedClient;
    	crashedClient = 0;
    }
    mCrashedProcesses.clear();
    
    emit allClientsRemoved();
}



/*!
  Searches for a client with the given process id. Returns null if no match
  found.
 */
TasClient* TasClientManager::findByProcessId(const QString& processId, bool includeSocketLess)
{
    if(processId.isNull() || processId.isEmpty()){
        return 0;
    }
    TasClient* match = 0;
    if(mClients.contains(processId)){
        if(mClients.value(processId)->socket() || includeSocketLess){
            match = mClients.value(processId);
        }
    }
    return match;
}

#ifdef Q_OS_SYMBIAN
TasClient* TasClientManager::findByApplicationUid(const QString applicationUid)
{
    if(applicationUid.isNull() || applicationUid.isEmpty()){
        return 0;
    }
    TasClient* match = 0;
    foreach (TasClient* app, mClients){
        if(app->applicationUid() == applicationUid){ 
            //do not return socketless clients 
            if(app->socket()){
                match = app;
                break;
            }
        }
    }
    return match;
}
#endif  

/*!
  Searches for a client with the given process id and removes it from the list. 
  Returns null if no match found.
 */
TasClient* TasClientManager::removeByProcessId(const QString& processId)
{
    if(processId.isNull() || processId.isEmpty()){
        return 0;
    }
    TasClient* app = 0;
    if(mClients.contains(processId)){
        app = mClients.take(processId);
    }
    return app;
}


/*!
  Searches for a client with the given application name. Returns null if no match
  found.
 */
TasClient* TasClientManager::findByApplicationName(const QString& applicationName, bool includeSocketLess)
{
    if(applicationName.isNull() || applicationName.isEmpty()){
        return 0;
    }
    TasClient* match = 0;
    int smallest = -1;
    foreach (TasClient* app, mClients){
        if(app->applicationName() == applicationName){ 
            //do not return socketless clients unless specifically requested
            if(includeSocketLess || app->socket()){
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

/*!
  Returns the client which was added last (most recent).
  Returns null if the list is empty.
*/
TasClient* TasClientManager::latestClient(bool includeSocketless)
{
    //    int pid = TasNativeUtils::pidOfActiveWindow(mClients.keys());
    int pid = TasNativeUtils::pidOfActiveWindow(mClients);
    if (pid != -1) {
        QString processId = QString::number(pid);
        if(mClients.contains(processId)){
            TasClient* app = mClients.value(processId);
            if(app->socket() || includeSocketless){
                return app;
            }
        }        
    }        
#ifdef Q_OS_SYMBIAN 
    return 0;
#else
    //find latest
    TasClient* match = 0;
    int smallest = -1;
    if(!mClients.empty()){
        foreach (TasClient* app, mClients){
            if(( includeSocketless || app->socket() ) && app->pluginType() == TAS_PLUGIN){
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
#endif
}

TasClient* TasClientManager::logMemClient()
{
    TasClient* match = 0;
    if(!mClients.empty()){
        foreach (TasClient* app, mClients){
            if(app->pluginType() == LOG_MEM_SRV){                
                if(!app->socket()){
                    removeByProcessId(app->processId());
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
  Returns the client in the crashed application list with given process id.
  Returns null if no process with given id is found in the list.
*/
TasClient* TasClientManager::findCrashedApplicationById(const QString& processId)
{
    TasClient* client = 0;
    if (mCrashedProcesses.contains(processId)){
        client = mCrashedProcesses[processId];
    }
    return client;
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
    \class TasClient
    \brief TasAppliction utility class for application details

    Stores all of the details for plugins that connect to the server.
    QProcess is also stored in case the application is started from 
    the server.
    
*/

TasClient::TasClient(const QString& processId)
{
    mProcessId = processId;
    mProcess = 0;
    mSocket = 0;
    mCreationTime.start();
}

TasClient::~TasClient()
{
    //running processes will not be deleted (causes kill)
    if(mProcess && mProcess->state() != QProcess::Running){
        delete mProcess;
        mProcess = 0;
    }
    mSocket = 0;
}

/*!
  Returns the process id associated with the client.
 */
const QString& TasClient::processId() const
{
    return mProcessId;
}

/*!
  Set the process associated with the client.
 */
void TasClient::setProcess(QProcess* process)
{
    mProcess = process;
    connect(mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
}

/*!
  Returns the process associated with the client.
  Can be null if the application was not started
  through the server.
 */
QProcess* TasClient::process()
{
    return mProcess;
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
        
    mSocket = 0;
    //no process remove the client from the list
    //will register again if so wants
    if(!mProcess){
        TasClientManager::instance()->removeMe(*this);
        deleteLater();
    }
}

void TasClient::closeConnection()
{
    if(mSocket){
        disconnect(mSocket, SIGNAL(socketClosed()), this, SLOT(disconnected()));    
        mSocket->closeConnection();
    }
}

QDateTime TasClient::crashTime()
{
	return mCrashTime;
}

/*!
  Set the socket connected to the client.
 */
void TasClient::setSocket(TasSocket* socket)
{
    mSocket = socket;
    connect(mSocket, SIGNAL(socketClosed()), this, SLOT(disconnected()));    
}

void TasClient::killProcess()
{
    if(mProcess){    

        if(mSocket){
            mSocket->closeConnection();
        }
        mSocket = 0;

        if(mProcess->state() != QProcess::NotRunning){
            //disconnect from processfinished since killing it
            stopProcessMonitor();
            mProcess->kill();
        }
        delete mProcess;
        mProcess = 0;
    }
}
 
/*!
  Monitoring process finish may cause problems in some situations like close.
 */
void TasClient::stopProcessMonitor()
{
    disconnect(mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
}

int TasClient::upTime()
{
    return mCreationTime.elapsed();
}

void TasClient::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED( exitCode );
    TasLogger::logger()->debug(
            "TasClient::processFinished: " + mApplicationName + ", exit status: " + exitStatus );

    TasClientManager::ClientError error;
    bool shouldCleanUp = true;
    if (QProcess::CrashExit == exitStatus) {
    	TasLogger::logger()->debug("Process crashed, moving to crashed list");
        TasClientManager* clientManager = TasClientManager::instance();
        error = clientManager->moveClientToCrashedList(mProcessId);
        if (!error) {
        	shouldCleanUp = false;
        }
        mCrashTime = QDateTime::currentDateTime();
        emit crashed();
    } 
    
    if (shouldCleanUp) {
        mApplicationName.clear();
        mProcessId.clear();
    }

    if(mSocket){
        disconnect(mSocket, SIGNAL(socketClosed()), this, SLOT(disconnected()));           
        mSocket = 0;
    }

    if (mProcess) {
        delete mProcess;
        mProcess = 0;
    }
}

bool TasClient::operator ==(const TasClient &client) const
{
    return processId() == client.processId();
}

void TasClient::setRegistered()
{
    emit registered(mProcessId);
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

ClientRemoveFilter::ClientRemoveFilter(TasCommandModel& model)
{
    mWaitTime = 3000;
    mKill = false;
    TasTarget* target = model.findTarget(APPLICATION_TARGET);    
    if(target){
        TasCommand* command = target->findCommand("Close");
        if(command){
            mProcessId = command->parameter("uid");    
            QString timeString = command->parameter("wait_time");
            if(!timeString.isEmpty()){
                mWaitTime = timeString.toInt() * 1000;
            }        
            if( command->parameter("kill") == "true"){
                mKill = true;
            }
        }
    }
}
ClientRemoveFilter::~ClientRemoveFilter()
{}

void ClientRemoveFilter::filterResponse(TasMessage& response)
{
    if(!mProcessId.isEmpty()){
        TasClientManager::CloseStatus status = TasClientManager::instance()->removeClient(mProcessId, mKill, mWaitTime);
        if(status == TasClientManager::Stalled){
            TasLogger::logger()->debug("ClientRemoveFilter::filterResponse application did not close ok!");
            QString errorMessage = "Application did not close in time.";
            if(mKill){
                errorMessage.append(" Application was killed.");
            }
            response.setErrorMessage(errorMessage);
        }
        else if(status == TasClientManager::Crashed){
            response.setErrorMessage("Application crashed during exit.");
        }
        else{
            TasLogger::logger()->debug("CloseAppService::stopApplication application closed successfully!");
            response.setIsError(false);
            response.setData(OK_MESSAGE);
        }
    }
}
