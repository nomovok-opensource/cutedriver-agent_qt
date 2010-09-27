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
 


#include <QByteArray>
#include <QTcpSocket>
#include <QDomElement>
#include <QDomDocument>
#include <QFile>

#include "taslogger.h"
#include "tascoreutils.h"
#include "testabilitysettings.h"
#include "servermonitor.h"

#ifdef Q_OS_SYMBIAN
#include <e32base.h>
_LIT( KQTasServerName, "qttasserver" );
#endif


const static QString SERVER_PATH = "qttasserver";

const static QString CLOSE_APP = "<TasCommands service=\"closeApplication\" name=\"\" id=\"\">" 
    "<Target TasId=\"Application\"><Command name=\"Close\" uid=\"0\"/></Target></TasCommands>";

const static QString SERVER_PING = "<TasCommands service=\"listApps\">" 
    "<Target TasId=\"Application\"><Command name=\"listApps\"/></Target></TasCommands>";

const static QString NOT_CONNECTED = "Down";

const static QString CONNECTED = "Connected";

const static QString NOT_RESPONDING = "Not responding";

const static QString RUNNING = "Running";

#ifdef Q_OS_SYMBIAN
const static QString SERVERINI =  "c:\\system\\data\\qttasserver.ini";
#endif

ServerMonitor::ServerMonitor(QObject* parent)
    :QObject(parent)
{
    mClient = new TasClient();
    connect(mClient, SIGNAL(error(const QString&)), this, SLOT(error(const QString&)));
    connect(mClient, SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
    connect(mClient, SIGNAL(serverResponse(const QString&)), this, SLOT(serverResponse(const QString&)));

    TasLogger::logger()->setLogFile("qttas_ui.log");         
    TasLogger::logger()->setLevel(DEBUG);                              
    TasLogger::logger()->clearLogFile();   
    
}

ServerMonitor::~ServerMonitor()
{
    delete mClient;
}

void ServerMonitor::serverState()
{
    mState = STATUS;
    emit serverDebug("Get server status..");    
    mClient->sendMessage(SERVER_PING);
}

void ServerMonitor::restartServer()
{
    emit beginMonitor();
    mState = RESTART;
    emit serverDebug("Restarting server...");
    emit serverDebug("Stopping server...");
    mClient->sendMessage(CLOSE_APP);
}

void ServerMonitor::startServer()
{
    emit beginMonitor();
    mState = START;
    emit serverDebug("Starting server....");
    QProcess::startDetached(SERVER_PATH);   
    emit serverDebug("Process started wait a bit...");
    QTimer::singleShot(3000, this, SLOT(serverState()));
}

void ServerMonitor::stopServer()
{        
    emit beginMonitor();
    mState = STOP;                               
    emit serverDebug("Stopping server...");
    mClient->sendMessage(CLOSE_APP);
    TasLogger::logger()->debug("ServerMonitor::stopServer");
}


void ServerMonitor::info(const QString& message)
{
    emit serverDebug(message);
}

void ServerMonitor::error(const QString& message)
{
    TasLogger::logger()->debug("ServerMonitor::error " + message);
    emit serverDebug(message);
    emit serverState(NOT_CONNECTED);
    if(mState != RESTART){
        emit stopMonitor();
    }
    if(mState == STOP || mState == RESTART){
        emit serverDebug("Connection was closed. Server appears to have stopped.");
        killServer();
        if(mState == RESTART){
            TasCoreUtils::wait(1000);
            startServer();
        }
    }
}

void ServerMonitor::serverResponse(const QString& message)
{
    TasLogger::logger()->debug("ServerMonitor::serverResponse " + message);
    if(mState == STOP || mState == RESTART){
        emit serverDebug("Close command ok server stopping.");    
    }
    else{
        QDomDocument doc("TasMessage");    
        if(mState == STATUS && doc.setContent(message)){
            emit serverDebug("Currently registered applications:");    
            QDomNodeList targets = doc.elementsByTagName (QString("object"));
            int count = targets.count();        
            for (int i = 0; i < count; i++){
                QDomElement target = targets.item(i).toElement();
                if(target.attribute("type") == "application"){
                    emit serverDebug(target.attribute("name"));        
                }
            }
        }
        else{
            emit serverDebug(message);    
        }
        emit serverState(RUNNING);                
        emit stopMonitor();
    }
}

void ServerMonitor::killServer()
{ 
    TasLogger::logger()->debug("ServerMonitor::killServer");
#ifdef Q_OS_SYMBIAN
    TFindProcess findProcess;
    TFullName processName;
    while ( findProcess.Next( processName ) == KErrNone )
        {
        if ( ( processName.Find( KQTasServerName ) != KErrNotFound ) )
            {
            RProcess process;
            TInt err = process.Open( findProcess );
            if( err == KErrNone)
                {
                if( process.ExitType() != EExitPending )
                    {
                    process.Close();
                    }
                else
                    {
                    process.Kill(0);
                    process.Close();
                    break;
                    }
                }              
            }
        }
#else
    //make sure connections are re opened 
    mClient->deleteLater();
    mClient = new TasClient();
    connect(mClient, SIGNAL(error(const QString&)), this, SLOT(error(const QString&)));
    connect(mClient, SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
    connect(mClient, SIGNAL(serverResponse(const QString&)), this, SLOT(serverResponse(const QString&)));
#endif
}

#ifdef Q_OS_SYMBIAN
void ServerMonitor::enablePluginLoad()
{  
    emit serverDebug("Attempting to enable plugin loading...");    
    QProcess process;
    process.start("TasHookActivator");
    if(process.waitForStarted()){
        emit serverDebug("Plugin enabler started successfully.");    
        if(process.waitForFinished()){            
            emit serverDebug("Plugin enabler executed successfully.");    
            emit serverDebug("Applications should now load the plugin.");    
        }
        else{
            emit serverDebug("Plugin enabler did not finish properly.");    
        }
    }
    else{
        emit serverDebug("Could not start enabler. " + process.errorString());            
    }
}

void ServerMonitor::setAutoStart(bool autostart)
{
    if(autostart){
        emit serverDebug("Setting autostart value on");    
        if(TestabilitySettings::settings()->setValue(AUTO_START, "on")){
            emit serverDebug("Setting autostart value on succeeded.");    
        }
        else{
            emit serverDebug("Setting autostart value on failed.");    
        }
    }
    else{
        emit serverDebug("Setting autostart value off");    
        if(TestabilitySettings::settings()->setValue(AUTO_START, "off")){
            emit serverDebug("Setting autostart value off succeeded.");    
        }
        else{
            emit serverDebug("Setting autostart value off failed.");    
        }
    }
}

bool ServerMonitor::autostartState()
{    
    return TasCoreUtils::autostart();
}

#endif



TasClient::TasClient()
{
    TasLogger::logger()->debug("TasClient::TasClient");
    mTimer.setSingleShot(true);
    mConnected = false;
    TasLogger::logger()->debug("TasClient::TasClient make socket");
    mSocket = new TasClientSocket(mServerConnection);
	connect(mSocket, SIGNAL(socketClosed()), this, SLOT(connectionClosed()));
    TasLogger::logger()->debug("TasClient::TasClient set responsehandler");
    mSocket->setResponseHandler(this);
    //connectToServer();
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));
    mSending = false;
    TasLogger::logger()->debug("TasClient::TasClient done");
}

TasClient::~TasClient()
{
    delete mSocket;
}

void TasClient::connectionClosed()
{
    TasLogger::logger()->debug("TasClient::connectionClosed");
    if(mConnected){
        mConnected = false;
        emit error("Connection was closed by server.");
    }
}

bool TasClient::connectToServer()
{
#if defined(TAS_NOLOCALSOCKET)
    mServerConnection.connectToHost(QT_SERVER_NAME, QT_SERVER_PORT);
#else
    mServerConnection.connectToServer(LOCAL_SERVER_NAME);
#endif   
    mConnected = mServerConnection.waitForConnected(3000);
    return mConnected;
}

void TasClient::sendMessage(const QString& message)
{
    if(mTimer.isActive()){
        emit error("Processing earlier message.");
    }
    else{        
        if(!mConnected){
            emit info("Not connected to server. Connecting...");
            connectToServer();
        }
        if(mConnected){
            emit info("Connection succesfull, sending message...");
            sendData(message);
        }
        else{
            emit error("Could not connect to server (maybe not running).");
        }
    }
}

void TasClient::sendData(const QString& message)
{
    TasLogger::logger()->debug("TasClient::sendData");
    mTimer.start(10000);
    mMessageId = qrand();        
    if(!mSocket->sendRequest(mMessageId, message)){
        mTimer.stop();
        mConnected = false;
        mServerConnection.close();      
        emit error("Socket not writable!");
    }
    TasLogger::logger()->debug("TasClient::sendData done");
}

void TasClient::serviceResponse(TasMessage& response)
{
    TasLogger::logger()->debug("TasClient::serviceResponse");
    if(response.messageId() == mMessageId){
        mTimer.stop();
        emit serverResponse(response.dataAsString());
    }    
    else{
        emit error("Server responded with an invalid message.");
    }
}

void TasClient::connectionTimeout()
{
    mServerConnection.close();
    mConnected = false;
    emit error("Server did not respond in time.");
}

