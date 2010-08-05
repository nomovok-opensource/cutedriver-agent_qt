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
 


#include "tasxmlwriter.h"
#include "testabilityservice.h"
#include "tascommandparser.h"
#include "closeappservice.h"
#include "objectservice.h"
#include "screenshotservice.h"
#include "uicommandservice.h"
#include "uistateservice.h"
#include "webkitcommandservice.h"
#include "recorderservice.h"
#include "taslogger.h"
#include "tasdatashare.h"
#include "taspluginloader.h"
#include "confservice.h"
#include "platformservice.h"
#include "infoservice.h"

TestabilityService *TestabilityService::mInstance = 0;

static const int SERVER_REGISTRATION_TIMEOUT = 12000;
static const int REGISTER_INTERVAL = 300;
static const int PAINT_EVENT_LIMIT = 10;


/*!
    \class TestabilityService
    \brief TestabilityService listens to ui commands and ui state requests from TasServer.
    
    TestabilityServiceable TasPlugins use the TestabilityService component to reqister to the TasServer. TestabilityService
    provides services to accept ui commands originating from the TasServer. 
            
    TasPlugins that want to use this component must implement the TestabilityServiceable interface.
    
    TestabilityService is implemented as a singleton.
    
*/
extern "C" TAS_EXPORT void qt_testability_init()
{
    if(qApp->type() == QApplication::Tty){
        return;
    }

    /* black listed apps */ 
    if(TestabilityUtils::isBlackListed()){
        return;
	}	

    /* If autostart on make sure server running */
    if(TestabilityUtils::autostart()){
        TasCoreUtils::startServer();
    }

    QVariant prop = qApp->property(PLUGIN_ATTR.toLatin1());
    if(prop.isValid() && prop.toBool()){
        return;
    }

    qApp->setProperty(PLUGIN_ATTR.toLatin1(), QVariant(true));

    QString id = QString::number(qApp->applicationPid());
    QString name = TestabilityUtils::getApplicationName();
    TestabilityService::instance();
}




/*!
    Constructs a new TestabilityService with \a parent.
 */
TestabilityService::TestabilityService(QObject* parent)
    : QObject(parent)
{        

    mMessageId = 0;
    mPaintEventCounter = 0;

    QString appName = TestabilityUtils::getApplicationName();
    TasLogger::logger()->setLogFile(appName+".log");                                                                                                                          
    TasLogger::logger()->setLevel(DEBUG);                              
    TasLogger::logger()->info("TestabilityService::TestabilityService initialized"); 
    mConnected = false;
    mRegistered = false;   
    mMarkedForDeletion = false;
       
    mPluginId = QString::number(qApp->applicationPid());

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToExit()));

    initializeServiceManager();
    initializeConnections();

    mRegisterTime.setSingleShot(true);
    connect(&mRegisterTime, SIGNAL(timeout()), this, SLOT(timeout()));
    loadStartUpParams(appName);

    mRegisterWatchDog.setSingleShot(true);

    //track paints and register after the first 300 millis pause
    qApp->installEventFilter(this);
    mPaintTracker.setSingleShot(true);
    connect(&mPaintTracker, SIGNAL(timeout()), this, SLOT(registerPlugin()));

    //After 5 secs force register
    mRegisterWatchDog.start(5000);
    connect(&mRegisterWatchDog, SIGNAL(timeout()), this, SLOT(registerPlugin()));

}

void TestabilityService::initializeConnections()
{
#if defined(TAS_NOLOCALSOCKET)
    mServerConnection = new QTcpSocket(this);
#else
    mServerConnection = new QLocalSocket(this);
#endif
    mSocket = new TasClientSocket(*mServerConnection, this);               
    connect(mSocket, SIGNAL(socketClosed()), this, SLOT(connectionClosed()));
    mSocket->setRequestHandler(mServiceManager);

}

TestabilityService* TestabilityService::instance()
{
    if(mInstance == 0){
        mInstance = new TestabilityService();
    }
    return mInstance;
}

void TestabilityService::closeApplication()
{
    TasLogger::logger()->debug("TestabilityService::closeApplication");
    prepareForDeletion();
    TasLogger::logger()->debug("TestabilityService::closeApplication quit app");
    qApp->quit();                
}

void TestabilityService::prepareForDeletion()
{
    if(!mMarkedForDeletion){
        mMarkedForDeletion = true;
        mRegisterTime.stop();
        mRegisterWatchDog.stop();
        mRegistered = false;
        if(mConnected){
            mConnected = false;       
            mSocket->closeConnection();     
        }
    }
}


/*!
    Destructor for TestabilityService
 */
TestabilityService::~TestabilityService()
{
    if(mServiceManager){
        delete mServiceManager;
        mServiceManager = 0;
    }
    if(mSocket){
        delete mSocket;
        mSocket = 0;
    }
    if(mServerConnection){
        delete mServerConnection;    
        mServerConnection = 0;
    }
}

/*!
     
    Register the TestabilityServiceable plugin to use the service.
    A register message is sent to the TasServer. The reqister
    message will include connection indentification that allows
    the server to send requests to the service and through to 
    the actual plugin.
    The sequence when registering is:
    1. Connect 
    2. Connected slot (listend to connected from socket) called send register
    3. emit registered
 
 */
void TestabilityService::registerPlugin()
{
    //make sure that no register attemps made if marked for deletion
    if(mMarkedForDeletion){
        return;
    }

    //remove paint tracking, paint tracking only on startup after this rely on the watchdog
    qApp->removeEventFilter(this);

    if(!mServerConnection->isWritable() && !mRegisterTime.isActive()){
        TasLogger::logger()->info("TestabilityService::registerPlugin connection device not writable maybe connection not initialized."); 
        connectionClosed();
    }

    if(!mRegistered && !mRegisterTime.isActive()){
        TasLogger::logger()->info("TestabilityService::registerPlugin not registered begin register process..."); 
        mRegisterWatchDog.stop();
        mRegisterTime.start(SERVER_REGISTRATION_TIMEOUT);
        connect(mServerConnection, SIGNAL(connected()), this, SLOT(sendRegisterMessage()));
#if defined(TAS_NOLOCALSOCKET)
        mServerConnection->connectToHost(QT_SERVER_NAME, QT_SERVER_PORT);
#else
        mServerConnection->connectToServer(LOCAL_SERVER_NAME);
#endif   
    }
}

void TestabilityService::sendRegisterMessage()
{  
    disconnect(mServerConnection, SIGNAL(connected()), this, SLOT(sendRegisterMessage()));

    QMap<QString, QString> attrs;
    attrs[PLUGIN_ID] = mPluginId;
    attrs[PLUGIN_NAME] = TestabilityUtils::getApplicationName();
    attrs[PLUGIN_TYPE] = TAS_PLUGIN;
#ifdef Q_OS_SYMBIAN
    quintptr uid = CEikonEnv::Static()->EikAppUi()->Application()->AppDllUid().iUid;
    attrs[APP_UID] = QString::number(uid);
#endif        
    QString message = makeReqisterMessage(COMMAND_REGISTER, attrs);

    mSocket->setResponseHandler(this);

    TasLogger::logger()->info("TestabilityService::reqisterServicePlugin send register message."); 
    mMessageId++;
    if(!mSocket->sendRequest(mMessageId, message)){
        TasLogger::logger()->error("TestabilityService::reqisterServicePlugin registering failed"); 
        mRegisterTime.stop();
        mSocket->closeConnection();
        connectionClosed();
    }
}

/*!
  Closed all connections and sets the app to not registered state.
*/
void TestabilityService::connectionClosed()
{
    TasLogger::logger()->error("TestabilityService::connectionClosed was closed");
    mRegistered = false;   
    mConnected = false;
    mRegisterTime.stop();

    //Proper fix needed at some point:
    //for some reason we need to reuse the old connections in symbian (seems to crash or freeze if not)
    //but in windows the old connections will not work at all and we need to recreate them     
#ifdef Q_OS_SYMBIAN
    mSocket->closeConnection();
#else
    if(!mMarkedForDeletion){   
        //make new connections
        delete mSocket;
        delete mServerConnection;
        initializeConnections();
    }
#endif

    mRegisterWatchDog.start(SERVER_REGISTRATION_TIMEOUT);     
    emit unRegistered();
}

void TestabilityService::serviceResponse(TasMessage& response)
{
    if(mMessageId != response.messageId()){
        TasLogger::logger()->error("TestabilityService::reqisterDone invalid message response");
        mSocket->closeConnection();
        connectionClosed();
    }
    else{
        TasLogger::logger()->info("TestabilityService::reqisterDone application registered");
        mRegisterTime.stop();
        mRegistered = true;
        //reset handler
        mSocket->setResponseHandler(0);
        emit registered();
    }
}

void TestabilityService::timeout()
{
    TasLogger::logger()->error("TestabilityService::timeout registering failed");        
    mSocket->closeConnection();
    connectionClosed();
}

void TestabilityService::aboutToExit()
{
    unReqisterServicePlugin();     
}

/*!
 
    Sends an unregister message to the TasServer.

    Only use this function if you really need to unregister.
    The default behaviour is to let the plugin destructor
    take care of this.
 
 */
void TestabilityService::unReqisterServicePlugin()
{       
    if(mRegistered){  
        QMap<QString, QString> attrs;
        attrs[PLUGIN_ID] = mPluginId;
        QString message = makeReqisterMessage(COMMAND_UNREGISTER, attrs);
        mMessageId++;
        mSocket->sendRequest(mMessageId, message);
        mRegistered = false;        
        emit unRegistered();
    }
    prepareForDeletion();
}




QString TestabilityService::makeReqisterMessage(QString command, QMap<QString,QString> attributes)
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

void TestabilityService::initializeServiceManager()
{
    mServiceManager = new TasServiceManager();
    mEventService = new EventService();
    mFixtureService = new FixtureService();
    //initialize service
    mServiceManager->registerCommand(mEventService);
    mServiceManager->registerCommand(mFixtureService);
    mServiceManager->registerCommand(new UiStateService());
    mServiceManager->registerCommand(new CloseAppService());    
    mServiceManager->registerCommand(new ObjectService());
    mServiceManager->registerCommand(new ScreenshotService());
    mServiceManager->registerCommand(new UiCommandService());
    mServiceManager->registerCommand(new RecorderService());
    mServiceManager->registerCommand(new ConfService());
    mServiceManager->registerCommand(new WebkitCommandService());
    mServiceManager->registerCommand(new InfoService());
}

void TestabilityService::loadStartUpParams(QString appName)
{
    TasDataShare dataShare;
    TasSharedData* data = dataShare.loadSharedData(appName);
    if(data){
        QStringList eventList = data->eventsToListen();
        QStringList signalList = data->signalsToListen();
        if(mEventService && !eventList.isEmpty()){
            mEventService->enableEvents(QString::number(qApp->applicationPid()), qApp, eventList);
            mEventService->addProcessStartEvent(data->creationTime());
        }
        if(mEventService && !signalList.isEmpty()){
            for (int i = 0; i < signalList.size(); i++){
                enableSignalTracking(signalList.at(i), data->creationTime().toString(DATE_FORMAT));
            }
        }
        delete data;
    }
}

void TestabilityService::enableSignalTracking(QString signal, QString timeStamp)
{
    TasCommandModel model;
    model.setService(FIXTURE);
    TasTarget& target = model.addTarget( TasCoreUtils::objectId( qApp ) );
    target.setType(TYPE_APPLICATION_VIEW);
    TasCommand& command = target.addCommand("Fixture");
    command.addParameter("plugin","tassignal");
    command.addParameter("method","enable_signal");
    command.addApiParameter(SIGNAL_KEY,signal);    
    command.addApiParameter(PROCESS_START_TIME,timeStamp);
    QString message;
    if(!mFixtureService->performFixture(model, message)){
        TasLogger::logger()->error("TestabilityService::enableSignalTracking failed. " + message);
    }
}

bool TestabilityService::eventFilter(QObject *target, QEvent *event)
{     
    Q_UNUSED(target);
    if( event->type() == QEvent::Paint){
        ++mPaintEventCounter;
        mPaintTracker.start(REGISTER_INTERVAL);        
    }
    if (mPaintEventCounter > PAINT_EVENT_LIMIT) {
        TasLogger::logger()->debug("TestabilityService::eventFilter Paint limit exceeded, forcing register");         
        registerPlugin();
    }

    return false;
}
