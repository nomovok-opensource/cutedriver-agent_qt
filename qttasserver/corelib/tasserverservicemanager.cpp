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

#include "tasserverservicemanager.h"

#include "tascommandparser.h"
#include "tasconstants.h"
#include "taslogger.h"
#include "version.h"

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

  Commands which are reqistered to the manager will be invoked on all 
  service requests untill on command consumed the request. This is done
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
        if(!targetClient){
            foreach(TasExtensionInterface* extension, mExtensions){            
                QByteArray data;
                if(extension->performCommand(commandModel, data)){
                    //remove possible client 
                    if(commandModel.service() == CLOSE_APPLICATION){
                        TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest remove client");
                        mClientManager->removeClient(commandModel.id());
                    }
                    TasResponse response(responseId, new QByteArray(data));
                    requester->sendMessage(response);
                    TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest: plat service done, returning");
                    return;
                }
            }
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
        ResponseWaiter* waiter = new ResponseWaiter(responseId, requester);
        bool needFragment = false;
        if(commandModel.service() == APPLICATION_STATE || commandModel.service() == FIND_OBJECT_SERVICE){
            foreach(TasExtensionInterface* traverser, mExtensions){
                QByteArray data = traverser->traverseApplication(targetClient->processId(), targetClient->applicationName());
                if(!data.isNull()){
                    waiter->appendPlatformData(data);
                    needFragment = true;
                }
            }
        }
        connect(waiter, SIGNAL(responded(qint32)), this, SLOT(removeWaiter(qint32)));
        reponseQueue.insert(responseId, waiter);
        if(needFragment){
            commandModel.addAttribute("needFragment", "true");
            targetClient->socket()->sendRequest(responseId, commandModel.sourceString(false));            
        }
        else{
            targetClient->socket()->sendRequest(responseId, commandModel.sourceString());            
        }

    }
    else{
        //check if platform specific handlers want to handle the request
        foreach(TasExtensionInterface* extension, mExtensions){            
            QByteArray data;
            if(extension->performCommand(commandModel, data)){
                TasLogger::logger()->debug("TasServerServiceManager::handleServiceRequest platform handler completed request.");
                TasResponse response(responseId, new QByteArray(data));
                requester->sendMessage(response);
                return;
            }
        }
        if(commandModel.service() == SCREEN_SHOT){
            ResponseWaiter* waiter = new ResponseWaiter(responseId, requester);
            connect(waiter, SIGNAL(responded(qint32)), this, SLOT(removeWaiter(qint32)));
            reponseQueue.insert(responseId, waiter);
            QStringList args;
            args << "-i" << QString::number(responseId) << "-a" << "screenshot";
            QProcess::startDetached("qttasutilapp", args);
        }
        else{            
            TasResponse response(responseId);
            response.setRequester(requester);
            performService(commandModel, response);
            //start app waits for register message and performs the response
            if( (commandModel.service() != START_APPLICATION && commandModel.service() != RESOURCE_LOGGING_SERVICE) || response.isError()){
                requester->sendMessage(response);
            }
        }
    }
}

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
    if(reponseQueue.contains(response.messageId())){
        reponseQueue.value(response.messageId())->sendResponse(response);
    }
    else{
        TasLogger::logger()->warning("TasServerServiceManager::serviceResponse unexpected response. Nobody interested!");
    }
}

void TasServerServiceManager::removeWaiter(qint32 responseId)
{
    //TasLogger::logger()->debug("TasServerServiceManager::removeWaiter remove " + QString::number(responseId));
    reponseQueue.remove(responseId);
}

QByteArray TasServerServiceManager::responseHeader()
{
    QString name = "qt";
#ifdef Q_OS_SYMBIAN   
    name = "symbian";
#endif
       
    QString header = "<tasMessage dateTime=\"" +
        QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz") + 
        "\" version=\""+TAS_VERSION+"\"><tasInfo id=\""+qVersion()+"\" name=\""+name+"\" type=\"sut\">";
    return header.toUtf8();
    
}

ResponseWaiter::ResponseWaiter(qint32 responseId, TasSocket* relayTarget, int timeout)
{
    mPlatformData = 0;
    mFilter = 0;
    mSocket = relayTarget;
    mResponseId = responseId;
    mWaiter.setSingleShot(true);    
    connect(&mWaiter, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(mSocket, SIGNAL(socketClosed()), this, SLOT(socketClosed()));
    mWaiter.start(timeout);
}

ResponseWaiter::~ResponseWaiter()
{
    mWaiter.stop();
    if(mFilter){
        delete mFilter;
    }
    if(mPlatformData){
        delete mPlatformData;
    }
}

void ResponseWaiter::appendPlatformData(QByteArray data)
{
    if(!mPlatformData){
        TasLogger::logger()->debug("ResponseWaiter::appendPlatformData make data container and add root");
        //make header for the document made from fragments
        mPlatformData = new QByteArray(TasServerServiceManager::responseHeader());
    }
    mPlatformData->append(data);
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
    TasLogger::logger()->debug("ResponseWaiter::sendResponse");
    mWaiter.stop();
    if(mFilter){
        mFilter->filterResponse(response);
    }
    if(mPlatformData && !mPlatformData->isEmpty()){
        TasLogger::logger()->debug("ResponseWaiter::sendResponse add plat stuf");
        response.uncompressData();
        mPlatformData->append(response.data()->data());
        mPlatformData->append(QString("</tasInfo></tasMessage>").toUtf8());
        response.setData(mPlatformData);
        //ownership transferred to response
        mPlatformData = 0;
    }
    //TasLogger::logger()->debug(response.dataAsString());
    if(!mSocket->sendMessage(response)){
        TasLogger::logger()->error("ResponseWaiter::sendResponse socket not writable!");
    }
    emit responded(mResponseId);
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
    if(!mSocket->sendMessage(response)){
        TasLogger::logger()->error("ResponseWaiter::timeout socket not writable!");
    }
    emit responded(mResponseId);
    deleteLater();
}

void ResponseWaiter::socketClosed()
{
    mWaiter.stop();
    TasLogger::logger()->error("ResponseWaiter::socketClosed. Connection to this waiter closed. Cannot respond. " );
    emit responded(mResponseId);
    deleteLater();
}









