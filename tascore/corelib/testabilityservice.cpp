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
#if !defined(NO_WEBKIT)
#include "webkitcommandservice.h"
#endif
#include "recorderservice.h"
#include "taslogger.h"
#include "tasdatashare.h"
#include "taspluginloader.h"
#include "confservice.h"
#include "platformservice.h"
#include "infoservice.h"
#include "eventservice.h"
#include "fixtureservice.h"
#include "findobjectservice.h"

const int SERVER_REGISTRATION_TIMEOUT = 12000;
const int REGISTER_INTERVAL = 300;
const int PAINT_EVENT_LIMIT = 10;


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
    // Ignore command line applications and the special launcher daemon in meego booster apps
    if(qApp->type() == QApplication::Tty || TestabilityUtils::getApplicationName() == "applauncherd.bin" || TestabilityUtils::getApplicationName() == "applifed.x" || 
       TestabilityUtils::getApplicationName() == "applifed") {
        return;
    }

    /* black listed apps */ 
    if(TestabilityUtils::isBlackListed()){
        return;
	}	

    QVariant prop = qApp->property(PLUGIN_ATTR);
    if(prop.isValid() && prop.toBool()){
        return;
    }

    /* If autostart on make sure server running */
    if(TestabilityUtils::autostart()){
        TasCoreUtils::startServer();
    }
    TestabilityLoader *loader = new TestabilityLoader();
    loader->load();
}

TestabilityLoader::TestabilityLoader()
{
    TasLogger::logger()->setLogFile(TestabilityUtils::getApplicationName()+".log");    
    TasLogger::logger()->setLevel(DEBUG); 
    mService = 0;
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(unload()));
}

void TestabilityLoader::load()
{
    //set prop for app that dll loaded
    qApp->setProperty(PLUGIN_ATTR, QVariant(true));
    mService = new TestabilityService();
    TasLogger::logger()->info("TestabilityLoader::intialized"); 
}

void TestabilityLoader::unload()
{    
    if(mService){
        qDebug("TestabilityLoader::remove testability");
        QVariant prop = qApp->property(CLOSE_REQUESTED);
        if(!prop.isValid() || !prop.toBool()){
            mService->unReqisterServicePlugin();
        }
        delete mService;
        mService = 0;
    }
    TasLogger::logger()->removeLogger();
    qDebug("TestabilityLoader::removed");
    deleteLater();
}



/*!
    Constructs a new TestabilityService with \a parent.
 */
TestabilityService::TestabilityService(QObject* parent)
    : QObject(parent)
{        
    mMessageId = 0;
    mPaintEventCounter = 0;
    mServiceManager = 0;

    mConnected = false;
    mRegistered = false;   
       
    mPluginId = QString::number(qApp->applicationPid());

    initializeServiceManager();
    initializeConnections();

    mRegisterTime.setSingleShot(true);
    connect(&mRegisterTime, SIGNAL(timeout()), this, SLOT(timeout()));
    loadStartUpParams(TestabilityUtils::getApplicationName());

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

/*!
    Destructor for TestabilityService
 */
TestabilityService::~TestabilityService()
{
    mRegisterTime.stop();
    mRegisterWatchDog.stop();
    mPaintTracker.stop();

    mEventService = 0;
    mFixtureService = 0;
    if(mSocket){
        disconnect(mSocket, SIGNAL(socketClosed()), this, SLOT(connectionClosed()));
        mSocket->clearHandlers();
        mSocket->closeConnection();     
        mSocket->deleteLater();
        mSocket = 0;
    }
    if(mServerConnection){
        mServerConnection->deleteLater();    
        mServerConnection = 0;
    }
    if(mServiceManager){
        delete mServiceManager;
        mServiceManager = 0;
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
    //close requested so do not register again
    QVariant prop = qApp->property(CLOSE_REQUESTED);
    if(prop.isValid() && prop.toBool()){
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
    // make new connections, deleting current once later. Deleting the current object inside the slot 
    // caused random crashes.
    mSocket->deleteLater();
    mServerConnection->deleteLater();
    initializeConnections();
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
    }
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
    mServiceManager->registerCommand(new CloseAppService());    
    mServiceManager->registerCommand(new ConfService());
    mServiceManager->registerCommand(new InfoService());
    mServiceManager->registerCommand(new ObjectService());
    mServiceManager->registerCommand(new ScreenshotService());
    mServiceManager->registerCommand(new UiCommandService());
#if !defined(NO_WEBKIT)
    mServiceManager->registerCommand(new WebkitCommandService());
#endif   
    mServiceManager->registerCommand(new UiStateService());
    mServiceManager->registerCommand(new RecorderService());    
    mServiceManager->registerCommand(new FindObjectService());    

    mEventService = new EventService();
    mServiceManager->registerCommand(mEventService);
    mFixtureService = new FixtureService();
    mServiceManager->registerCommand(mFixtureService);
}

void TestabilityService::loadStartUpParams(QString appName)
{
    TasLogger::logger()->error("TestabilityService::loadStartUpParams for app: " + appName);
    TasDataShare dataShare;
    QString errMsg = "";
    TasSharedData* data = dataShare.loadSharedData(appName, errMsg);
    if(data){
        TasLogger::logger()->error("TestabilityService::loadStartUpParams data");
        QStringList eventList = data->eventsToListen();
        QStringList signalList = data->signalsToListen();
        if(mEventService && !eventList.isEmpty()){
            TasLogger::logger()->error("TestabilityService::loadStartUpParams enable events: " + eventList.join(";"));
            mEventService->enableEvents(QString::number(qApp->applicationPid()), qApp, eventList);
            mEventService->addProcessStartEvent(data->creationTime());
        }
        if(mEventService && !signalList.isEmpty()){
            TasLogger::logger()->error("TestabilityService::loadStartUpParams listen signals: " + signalList.join(";"));
            for (int i = 0; i < signalList.size(); i++){
                enableSignalTracking(signalList.at(i), data->creationTime().toString(DATE_FORMAT));
            }
        }
        delete data;
    }
    else {

        TasLogger::logger()->error("TestabilityService::loadStartUpParams no data error:" + errMsg);
    }
}

void TestabilityService::enableSignalTracking(QString signal, QString timeStamp)
{
    //need to make a commandmodel for the fixture
    TasCommandModel* model = TasCommandModel::createModel();
    model->addDomAttribute("service", FIXTURE);
    TasTarget& target = model->addTarget();
    target.addDomAttribute("TasId", TasCoreUtils::objectId( qApp ));
    target.addDomAttribute("type", TYPE_APPLICATION_VIEW);
    TasCommand& command = target.addCommand();
    command.addDomAttribute("name", "Fixture");
    command.addDomAttribute("plugin","tassignal");
    command.addDomAttribute("method","enable_signal");
    command.addApiParameter(SIGNAL_KEY,signal, "QString");    
    command.addApiParameter(PROCESS_START_TIME,timeStamp, "QString");
    QString message;
    if(!mFixtureService->performFixture(*model, message)){
        TasLogger::logger()->error("TestabilityService::enableSignalTracking failed. " + message);
    }
    delete model;
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
