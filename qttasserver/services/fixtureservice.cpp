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
 

#include <QHash>

#include <tasqtdatamodel.h>

#include "fixtureservice.h"
#include <taslogger.h>
#include <tascommandparser.h>


FixtureService::FixtureService()
{
    mPluginLoader = new TasPluginLoader();    
	mInitialized = false;
    mTimer = new QTimer();
    mTimer->setInterval(1);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(delayedEvent()));
}


FixtureService::~FixtureService()
{
    delete mPluginLoader;
    delete mTimer;
}

bool FixtureService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        performServerFixture(model, response);
        TasLogger::logger()->debug("FixtureService::executeService done");
        return true;
    }
    else{
        return false;
    }
}

void FixtureService::performServerFixture(TasCommandModel &model, TasResponse& response)
{
    TasLogger::logger()->debug("FixtureService::performServerFixture");
	if(!mInitialized){
		mPluginLoader->initializeFixturePlugins();
		mInitialized = true;
	}
	
    if(!model.isAsynchronous()){
        QString message = "";
        if(performFixture(model, message)){
            response.setData(message);
            response.setIsError(false);
        }
        else{
            response.setErrorMessage(message);
        }
    }
    else{
        //needs some refactoring to avoid double parse...(luckily small docs..)
        commandQueue.enqueue(TasCommandParser::parseCommandXml(model.sourceString()));            
        mTimer->start();            
    }
}

bool FixtureService::performFixture(TasCommandModel& model, QString& message)
{
    TasLogger::logger()->debug("FixtureService::performFixture");
    QListIterator<TasTarget*> i(model.targetList());
    bool result = false;
    message = "";
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();
        TasCommand* fixture = commandTarget->findCommand("Fixture");
        if(fixture){
            QString plugin = fixture->parameter("plugin");
            QString method = fixture->parameter("method");
            TasLogger::logger()->debug("FixtureService::performFixture load fixture");
            TasFixturePluginInterface* fixturePlugin = mPluginLoader->loadFixtureInterface(plugin);
            if(fixturePlugin){
                TasLogger::logger()->debug("FixtureService::performFixture fixture loaded");
                QHash<QString,QString> parameters = fixture->getApiParameters();
                parameters.insert(OBJECT_TYPE, NULL_TYPE);
                TasLogger::logger()->debug("FixtureService::performFixture execute fixture");
                result = fixturePlugin->execute(0, method, parameters, message);
            }
            else{
                message = "No fixture plugin found for the given id or path: " + plugin;
            }
            break;
        }
        else{
            message = PARSE_ERROR;
        }
    }
    return result;
}



/*!
    Perform delayed events.
*/
void FixtureService::delayedEvent()
{    
    TasCommandModel* commands = commandQueue.dequeue();
    QString message;
    if(!performFixture(*commands, message)){
        TasLogger::logger()->error(message);
    }    
    delete commands;
    if(commandQueue.isEmpty()){
        mTimer->stop();
    }
}
