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
#include <tasxmlwriter.h>
#include <tascommandparser.h>
#include <taslogger.h>

#include "tasqtmemlogservice.h"
#include "resourceloggingservice.h"

static const int SERVER_REGISTRATION_TIMEOUT = 12000;
static const QString SRV_NAME = "qttasmemlog_srv";
 
TasQtMemLogService::TasQtMemLogService(QObject* parent)
    : QObject(parent)
{        
    mMessageId = 0;
    mDoNotReconnect = false;
    TasLogger::logger()->setLogFile(SRV_NAME+".log");
    TasLogger::logger()->setLevel(DEBUG);                              
    mConnected = false;
    mRegistered = false;   
    mServiceManager = new TasServiceManager();    
    mServiceManager->registerCommand(new ResourceLoggingService());
    initializeConnections();
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(unReqisterServicePlugin()));
    connect(&mRegisterTime, SIGNAL(timeout()), this, SLOT(timeout()));
  
}

void TasQtMemLogService::initializeConnections()
{
#if defined(TAS_NOLOCALSOCKET)
    mServerConnection = new QTcpSocket(this);
#else
    mServerConnection = new QLocalSocket(this);
#endif
    mSocket = new TasClientSocket(*mServerConnection, this);               
    connect(mSocket, SIGNAL(socketClosed()), this, SLOT(connectionClosed()));
    mSocket->setRequestHandler(mServiceManager);

    mRegisterTime.setSingleShot(true);
    mRegisterTime.start(SERVER_REGISTRATION_TIMEOUT);

    connect(mServerConnection, SIGNAL(connected()), this, SLOT(sendRegisterMessage()));
#if defined(TAS_NOLOCALSOCKET)
    mServerConnection->connectToHost(QT_SERVER_NAME, QT_SERVER_PORT);
#else
    mServerConnection->connectToServer(LOCAL_SERVER_NAME);
#endif   
}

/*!
    Destructor for TasQtMemLogService
 */
TasQtMemLogService::~TasQtMemLogService()
{
    if(mServiceManager){
        delete mServiceManager;
        mServiceManager = 0;
    }
    cleanConnections();
}

void TasQtMemLogService::cleanConnections()
{
    if(mSocket){
        delete mSocket;
        mSocket = 0;
    }
    if(mServerConnection){
        delete mServerConnection;    
        mServerConnection = 0;
    }
}

void TasQtMemLogService::sendRegisterMessage()
{  
    disconnect(mServerConnection, SIGNAL(connected()), this, SLOT(sendRegisterMessage()));

    if(mRegistered){
        return;
    }

    QMap<QString, QString> attrs;
    attrs[PLUGIN_ID] = QString::number(QCoreApplication::instance()->applicationPid());
    attrs[PLUGIN_NAME] = SRV_NAME;
    attrs[PLUGIN_TYPE] = LOG_MEM_SRV;
    QString message = makeReqisterMessage(COMMAND_REGISTER, attrs);
    mSocket->setResponseHandler(this);

    TasLogger::logger()->info("TasQtMemLogService::reqisterServicePlugin send register message."); 
    mMessageId++;
    if(!mSocket->sendRequest(mMessageId, message)){
        TasLogger::logger()->error("TasQtMemLogService::reqisterServicePlugin registering failed"); 
        mRegisterTime.stop();
        mSocket->closeConnection();
        mDoNotReconnect = true;
        connectionClosed();
        //no use if no connection to qttasserver
        QCoreApplication::intance()->quit();
    }
}

/*!
  Closed all connections and sets the app to not registered state.
*/
void TasQtMemLogService::connectionClosed()
{
    TasLogger::logger()->error("TasQtMemLogService::connectionClosed was closed");
    mRegistered = false;   
    mConnected = false;
    cleanConnections();
    if(!QCoreApplication::closingDown() && !mDoNotReconnect){
        initializeConnections();
    }
}


void TasQtMemLogService::serviceResponse(TasMessage& response)
{
    if(mMessageId != response.messageId()){
        TasLogger::logger()->error("TasQtMemLogService::reqisterDone invalid message response");
        mSocket->closeConnection();
        connectionClosed();
        mRegistered = false;
    }
    else{
        mRegistered = true;
        TasLogger::logger()->info("TasQtMemLogService::reqisterDone application registered");
        mRegisterTime.stop();
        mSocket->setResponseHandler(0);
    }
}

void TasQtMemLogService::timeout()
{
    TasLogger::logger()->error("TasQtMemLogService::timeout registering failed");        
    mSocket->closeConnection();
    connectionClosed();
    QCoreApplication::instance()->quit();
}

/*!
 
    Sends an unregister message to the TasServer.

    Only use this function if you really need to unregister.
    The default behaviour is to let the plugin destructor
    take care of this.
 
 */
void TasQtMemLogService::unReqisterServicePlugin()
{       
    mDoNotReconnect = true;

    if(mRegistered){  
        TasLogger::logger()->info("TasQtMemLogService::unReqisterServicePlugin");
        QMap<QString, QString> attrs;
        attrs[PLUGIN_ID] = QString::number(QCoreApplication::instance()->applicationPid());
        QString message = makeReqisterMessage(COMMAND_UNREGISTER, attrs);
        mMessageId++;
        mSocket->sendRequest(mMessageId, message);
        mRegistered = false;        
    }
}

QString TasQtMemLogService::makeReqisterMessage(QString command, QMap<QString,QString> attributes)
{    
    QString xml;
    
    QTextStream stream(&xml, QIODevice::WriteOnly);
    TasXmlWriter xmlWriter(stream);
    
    QMap<QString, QString> attrs;
    attrs[COMMAND_SERVICE] = REGISTER; 
    xmlWriter.openElement(COMMAND_ROOT, attrs);
    attrs.clear();
    attrs[COMMAND_TARGET_ID] = APPLICATION_TARGET;
    xmlWriter.openElement(COMMAND_TARGET, attrs);

    attributes[COMMAND_TYPE_NAME] = command;
    xmlWriter.openElement(COMMAND_TYPE, attributes);

    xmlWriter.closeElement(COMMAND_TYPE);
    xmlWriter.closeElement(COMMAND_TARGET);
    xmlWriter.closeElement(COMMAND_ROOT);   
    return xml;
}


