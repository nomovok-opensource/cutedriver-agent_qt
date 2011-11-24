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
 

                      
#include <QMutableListIterator>
#include <QtPlugin>
#include <QLibrary>
#include <QLibraryInfo>
#include <QDir>
#include <QPluginLoader>
#include <QProcess>
#include <QCoreApplication>

#include "tasserverservicemanager.h"

#include "tascommandparser.h"
#include "tasconstants.h"
#include "taslogger.h"
#include "version.h"
#include "tasnativeutils.h"

const char* const RESPONSE_HEADER = "<tasMessage version=\"%1\"><tasInfo id=\"%2\" name=\"%3\" type=\"sut\">";
const char* const RESPONSE_FOOTER = "</tasInfo></tasMessage>";
#ifdef Q_OS_SYMBIAN   
const char* const ENV_NAME = "symbian";
#else
const char* const ENV_NAME = "qt";
#endif    

/*!
  \class TasServerServiceManager
  \brief TasServerServiceManager manages the service commands used by qttasserver.
    
  TasServerServiceManager is the manager for the commands in the service architecture
  used by qttasserver. The service requests are implemented using a relatively
  standard form of the chain of responsibility pattern. TasServerServiceManager class
  takes care of the command execution and management. The command implementations
  only need to concern with the actual command implementation. 
  
  The difference to TasServiceManager is that a waiter is started for the responses
  which originate from the connected plugins.

  Commands which are reqistered to the manager will be invoked on all   service requests untill on command consumed the request. This is done
  by returning "true" from the execute method.

*/

/*!
  Construct a new TasServerServiceManager
*/
TasServerServiceManager::TasServerServiceManager(QObject *parent)
    :QObject(parent)
{
    mClientManager = TasClientManager::instance();
    loadExtensions();
}

/*!
  Destructor. Destroys all of the reqistered commands.
*/
TasServerServiceManager::~TasServerServiceManager()
{
    qDeleteAll(mCommands);
    mCommands.clear();
    mExtensions.clear();
    mResponseQueue.clear();
    mHeader.clear();
}

/*!
  Servers servicemanager first looks for a client to relay the message to. If none found then use the servers command chain
  to handle the service request.
*/
void TasServerServiceManager::handleServiceRequest(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId)
{
    TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest: " + commandModel.service() + ": " + commandModel.id());
    TasClient* targetClient = 0;
    if (!commandModel.id().isEmpty() && commandModel.id() != "1"){        
        targetClient = mClientManager->findByProcessId(commandModel.id());

        //no registered client check for platform specific handles for the process id
        if(!targetClient && extensionHandled(commandModel, requester, responseId)){            
            return;        
        }

        if(!targetClient){
            TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest: no target client send error...");
            TasResponse response(responseId);
            response.setIsError(true);
            response.setErrorMessage("The application with Id " + commandModel.id() + " is no longer available.");
            requester->sendMessage(response);
            return;
        }

    }

    if(!targetClient && (commandModel.service() == APPLICATION_STATE || commandModel.service() == SCREEN_SHOT 
                         || commandModel.service() == FIND_OBJECT_SERVICE)){
        targetClient = mClientManager->findClient(commandModel);
    }
    else if (commandModel.service() == RESOURCE_LOGGING_SERVICE){
        targetClient = mClientManager->logMemClient();
    }
    else{
    }

    if(targetClient){
        TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest client is: " + targetClient->processId() +
                                   "," + targetClient->applicationName());
        int timeout = 10000;
        if(!commandModel.parameter("plugin_timeout").isEmpty()){
            TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest set timeout " + commandModel.parameter("plugin_timeout"));
            timeout = commandModel.parameter("plugin_timeout").toInt();
        }

        ResponseWaiter* waiter = new ResponseWaiter(responseId, requester, timeout);
        if(commandModel.service() == CLOSE_APPLICATION){
            waiter->setResponseFilter(new CloseFilter(commandModel));        
        }
        connect(waiter, SIGNAL(responded(qint32)), this, SLOT(removeWaiter(qint32)));
        mMutex.lock();
        mResponseQueue.insert(responseId, waiter);
        mMutex.unlock();
        if(commandModel.service() == APPLICATION_STATE || commandModel.service() == FIND_OBJECT_SERVICE){
            commandModel.addDomAttribute("needFragment", "true");
            //send request for qt uistate to client
            QWeakPointer<ResponseWaiter> rWaiter(waiter);
            targetClient->socket()->sendRequest(responseId, commandModel.sourceString(false));                        
            //in the meantime process native
            getNativeUiState(waiter, commandModel);
            rWaiter.clear();
        }
        else{
            //can respond as soon as response from qt side
            waiter->okToRespond();
            targetClient->socket()->sendRequest(responseId, commandModel.sourceString());            
        }
    }        
    else{
        handleClientLess(commandModel, requester, responseId);
    }
    //https://bugreports.qt.nokia.com/browse/QTBUG-21928
    QCoreApplication::instance()->processEvents(QEventLoop::DeferredDeletion);
    TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest: done " + commandModel.service());
}

void TasServerServiceManager::getNativeUiState(QWeakPointer<ResponseWaiter> waiter, TasCommandModel& commandModel)
{
    foreach(TasExtensionInterface* traverser, mExtensions){
        QByteArray data = traverser->traverseApplication(commandModel);
        if(!data.isNull()){
            if(!waiter.isNull()){
                waiter.data()->appendPlatformData(data);
            }
            data.clear();
        }
        
    }
#ifdef Q_OS_SYMBIAN   
    QByteArray vkbData; 
    if(appendVkbData(commandModel, vkbData)){
        if(!waiter.isNull()){
            waiter.data()->appendPlatformData(vkbData);
            vkbData.clear();
        }
    }
#endif
    if(!waiter.isNull()){
        waiter.data()->okToRespond();
    }
}
void TasServerServiceManager::handleClientLess(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId)
{
    //check if platform specific handlers want to handle the request
    if(extensionHandled(commandModel, requester, responseId)){
        return;
    }
    if(commandModel.service() == SCREEN_SHOT){
        ResponseWaiter* waiter = new ResponseWaiter(responseId, requester);
        connect(waiter, SIGNAL(responded(qint32)), this, SLOT(removeWaiter(qint32)));
        mResponseQueue.insert(responseId, waiter);
        QStringList args;
        args << "-i" << QString::number(responseId) << "-a" << "screenshot";
        QProcess::startDetached("qttasutilapp", args);
    }
    else{
        //try to detect pc side connection breaks
        QWeakPointer<TasSocket> socketSafe = QWeakPointer<TasSocket>(requester);
        TasResponse response(responseId);
        response.setRequester(requester);
        performService(commandModel, response);
        if(!socketSafe.isNull() && (commandModel.service() != RESOURCE_LOGGING_SERVICE || response.isError())){
            requester->sendMessage(response);
        }
        else{
            TasLogger::logger()->warning("TasServerServiceManager::handleClientLess connection was broken!");
        }
    }
}

/*!
 * Some operation require custom platform specific handling. Will return true is handled by a special plugin.
 */
bool TasServerServiceManager::extensionHandled(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId)
{
    bool handled = false;
    foreach(TasExtensionInterface* extension, mExtensions){            
        QByteArray data;
        if(commandModel.service() == APPLICATION_STATE || commandModel.service() == FIND_OBJECT_SERVICE){
            QByteArray uiState = extension->traverseApplication(commandModel);
            if(!uiState.isEmpty()){
                handled = true;
#ifdef Q_OS_SYMBIAN   
                appendVkbData(commandModel,uiState);
#endif
            }
            data = QString(RESPONSE_HEADER).arg(TAS_VERSION).arg(qVersion()).arg(ENV_NAME).toUtf8();
            data.append(uiState);
            data.append(QString(RESPONSE_FOOTER).toUtf8());
        }        
        else {
            handled = extension->performCommand(commandModel, data);
        }
        if(handled){
            TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest platform handler completed request.");
            //make sure the pid is removed from the started apps list
            if(commandModel.service() == CLOSE_APPLICATION){
                bool ok;
                quint64 pid = commandModel.id().toULongLong(&ok);  
                if(ok) TasClientManager::instance()->removeStartedPid(pid);
                    
            }
            TasResponse response(responseId, data);
            requester->sendMessage(response);
            break;
        }
    }
    return handled;
}

#ifdef Q_OS_SYMBIAN   
/*!
 * Look for a special vkb and added if present and needed.
 */
bool TasServerServiceManager::appendVkbData(TasCommandModel& commandModel, QByteArray& data)
{
    TasLogger::logger()->debug("TasServerServiceManager::appendVkbData");
    bool appended = false;
    if( commandModel.name() != PENINPUT_SERVER && (commandModel.service() == FIND_OBJECT_SERVICE || commandModel.service() == APPLICATION_STATE) ){
        TasClient* targetClient = mClientManager->findByApplicationName(PENINPUT_SERVER);
        if(targetClient){
            TasLogger::logger()->debug("TasServerServiceManager::appendVkbData peninputserver found");
            bool requestVkb = false;
            if(commandModel.service() == FIND_OBJECT_SERVICE){
                foreach(TasTarget* target, commandModel.targetList()){
                    TasTargetObject *targetObj = target->targetObject();   
                    if(targetObj->className().contains(VKB_IDENTIFIER)){
                        requestVkb = true;
                        break;
                    }
                }
            }
            if(requestVkb || commandModel.service() == APPLICATION_STATE){
                commandModel.addDomAttribute("needFragment", "true");
                TasMessage reply;
                if(targetClient->socket()->syncRequest(qrand(), commandModel.sourceString(false), reply)){
                    if(!reply.data().isEmpty()){
                        TasLogger::logger()->debug("TasServerServiceManager::appendVkbData append vkb data");
                        appended = true;
                        data.append(reply.data());
                    }
                }
            }
        }
    }
    return appended;
}
#endif


void TasServerServiceManager::loadExtensions()
{
    TasLogger::logger()->debug("TasServerServiceManager::loadPlatformTraversers");
    QString pluginDir = "tasextensions";
    QStringList plugins = mPluginLoader.listPlugins(pluginDir);
    TasLogger::logger()->debug("TasServerServiceManager::loadPlatformTraversers plugins found: " + plugins.join("'"));
    QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath) + "/"+pluginDir;
    for (int i = 0; i < plugins.count(); ++i) {
        QString fileName = plugins.at(i);
        QString filePath = QDir::cleanPath(path + QLatin1Char('/') + fileName);
        if(QLibrary::isLibrary(filePath)){
            loadExtension(filePath);
        }
    }

}



/*!
  Try to load a plugin from the given path. Returns null if no plugin loaded.
*/
void TasServerServiceManager::loadExtension(const QString& filePath)
{
    TasExtensionInterface* interface = 0; 
    QObject *plugin = mPluginLoader.loadPlugin(filePath);
    if(plugin){
        interface = qobject_cast<TasExtensionInterface *>(plugin);        
        if (interface){
            TasLogger::logger()->debug("TasServerServiceManager::loadTraverser added a traverser");
            mExtensions.append(interface);
        }
        else{
            TasLogger::logger()->warning("TasServerServiceManager::loadTraverser could not cast to TasApplicationTraverseInterface");
        }
    }    
}



void TasServerServiceManager::serviceResponse(TasMessage& response)
{
    QMutexLocker locker(&mMutex);
    if(mResponseQueue.contains(response.messageId())){
        mResponseQueue.value(response.messageId())->sendResponse(response);
    }
    else{
        TasLogger::logger()->warning("TasServerServiceManager::serviceResponse unexpected response. Nobody interested!");
    }
}

void TasServerServiceManager::removeWaiter(qint32 responseId)
{
    QMutexLocker locker(&mMutex);
    TasLogger::logger()->debug("TasServerServiceManager::removeWaiter remove " + QString::number(responseId));
    mResponseQueue.remove(responseId);
    TasLogger::logger()->debug("TasServerServiceManager::removeWaiter response count " + QString::number(mResponseQueue.count()));
}

ResponseWaiter::ResponseWaiter(qint32 responseId, TasSocket* relayTarget, int timeout)
{
    mFilter = 0;
    mPluginResponded = false;
    mCanRespond = false;
    mSocket = QWeakPointer<TasSocket>(relayTarget);
    mResponseId = responseId;
    mWaiter.setSingleShot(true);    
    connect(&mWaiter, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(mSocket.data(), SIGNAL(socketClosed()), this, SLOT(socketClosed()));
    mWaiter.start(timeout);
}

ResponseWaiter::~ResponseWaiter()
{
    mWaiter.stop();
    if(mFilter){
        delete mFilter;
    }
    if(!mPlatformData.isEmpty()) mPlatformData.clear();
}

void ResponseWaiter::okToRespond()
{
    mCanRespond = true;
    if(mPluginResponded){
        sendMessage();
    }   
}

void ResponseWaiter::cleanup()
{
    disconnect(&mWaiter, 0, this, 0);
    disconnect(mSocket.data(), 0, this, 0);    
}

void ResponseWaiter::appendPlatformData(const QByteArray& data)
{
    if(mPlatformData.isEmpty()){
        //TasLogger::logger()->debug("ResponseWaiter::appendPlatformData make data container and add root");
        //make header for the document made from fragments
        mPlatformData = QString(RESPONSE_HEADER).arg(TAS_VERSION).arg(qVersion()).arg(ENV_NAME).toUtf8();
    }
    mPlatformData.append(data);
}

/*!
  If set the response will be passed on to the filter before sent to the
  requesting socket. Ownership is taken.
*/
void ResponseWaiter::setResponseFilter(ResponseFilter* filter)
{
    mFilter = filter;
}

void ResponseWaiter::sendResponse(TasMessage& response)
{
    //TasLogger::logger()->debug("ResponseWaiter::sendResponse");
    mPluginResponded = true;
    mWaiter.stop();
    mMessageToSend = response;
    //native stuff not ready
    if(!mCanRespond) return;
    sendMessage();
}

void ResponseWaiter::sendMessage(){
    if(mFilter){
        mFilter->filterResponse(mMessageToSend);
    }
    if(!mPlatformData.isEmpty()){
        //TasLogger::logger()->debug("ResponseWaiter::sendResponse add plat stuf");
        mMessageToSend.uncompressData();
        mPlatformData.append(mMessageToSend.data().data());
        mPlatformData.append(QString(RESPONSE_FOOTER).toUtf8());
        mMessageToSend.setData(mPlatformData);
    }
    if(mSocket.isNull()){
        TasLogger::logger()->error("ResponseWaiter::sendResponse socket no longer available!");
    }
    else{
        if(!mSocket.data()->sendMessage(mMessageToSend)){
            TasLogger::logger()->error("ResponseWaiter::sendResponse socket not writable!");
        }
    }
    emit responded(mResponseId);
    cleanup();
    deleteLater();
}

void ResponseWaiter::timeout()
{
    TasLogger::logger()->error("ResponseWaiter::timeout for message: " + QString::number(mResponseId));
    TasResponse response(mResponseId);
    response.setErrorMessage("Connection to plugin timedout!");
    if(mFilter){
        mFilter->filterResponse(response);
    }
    if(mSocket.isNull()){
        TasLogger::logger()->error("ResponseWaiter::timeout socket no longer available!");
    }
    else{
        if(!mSocket.data()->sendMessage(response)){
            TasLogger::logger()->error("ResponseWaiter::timeout socket not writable!");
        }
    }
    emit responded(mResponseId);
    cleanup();
    deleteLater();
}

void ResponseWaiter::socketClosed()
{
    mWaiter.stop();
    TasLogger::logger()->error("ResponseWaiter::socketClosed. Connection to this waiter closed. Cannot respond. " );
    emit responded(mResponseId);
    cleanup();
    deleteLater();
}

CloseFilter::CloseFilter(TasCommandModel& model)
{
    mPassThrough = true;
    mWaitTime = 3000;
    mKill = false;
    TasTarget* target = model.findTarget(APPLICATION_TARGET);    
    if(target){
        TasCommand* command = target->findCommand("Close");
        if(command){
            bool ok;
            mPid = command->parameter("uid").toULongLong(&ok, 10);     
            if(ok){
                mPassThrough = false;
            }
            QString timeString = command->parameter("wait_time");
            if(!timeString.isEmpty()){
                mWaitTime = timeString.toInt() * 1000;
            }        
            if( command->parameter("kill") == "true"){
                mKill = true;
            }
        }
        command = 0;
        target = 0;
    }
}
CloseFilter::~CloseFilter()
{}

void CloseFilter::filterResponse(TasMessage& response)
{
    if(mPassThrough) return;
    
    TasLogger::logger()->debug("CloseFilter::filterResponse");

    //we need to check that the app really closed
    int errorCode = 0;
    int totalSleep = 0;
    int wait = 200;
    QString errorMessage;
    bool didNotCloseInTime = false;

    while(!TasNativeUtils::processExitStatus(mPid, errorCode)){
        TasCoreUtils::wait(wait);        
        totalSleep+=wait;
        if(totalSleep >= mWaitTime){
            TasLogger::logger()->debug("CloseFilter::filterResponse application did not close in time!");
            errorMessage = "Application did not close in time.";
            didNotCloseInTime = true;
            if(mKill){
                TasNativeUtils::killProcess(mPid);                     
            }
            break;
        }
    }
    TasLogger::logger()->debug("CloseFilter::filterResponse exit code: " + QString::number(errorCode));
    //make sure the client is removed
    TasClientManager::instance()->removeClient(QString::number(mPid), false);

    if(!didNotCloseInTime && errorCode != 0){
        errorMessage = "Application exited with code " + QString::number(errorCode);
    }
    if(!errorMessage.isEmpty()){
        response.setErrorMessage(errorMessage);
    }
}












