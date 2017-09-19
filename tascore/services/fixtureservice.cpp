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



#include <QApplication>

#include "fixtureservice.h"
#include "taslogger.h"
#include "tascommandparser.h"

/*!
  \class FixtureService
  \brief FixtureService invokes fixtures

*/

FixtureService::FixtureService()
{
    mInitialized = false;
    mPluginLoader = new TasPluginLoader();
    mTimer.setInterval(50);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(delayedEvent()));
}

FixtureService::~FixtureService()
{
    mTimer.stop();
    delete mPluginLoader;
    while (!commandQueue.isEmpty()){
        delete commandQueue.takeFirst();
    }
}

bool FixtureService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){

        if(!model.isAsynchronous()){
            QString message = "";
            if(!performFixture(model, message)){
                response.setIsError(true);
            }
            response.setData(message);
        }
        else{
            //needs some refactoring to avoid double parse...(luckily small docs..)
            commandQueue.enqueue(TasCommandModel::makeModel(model.sourceString()));
            mTimer.start();
        }
        return true;
    }
    else{
        return false;
    }
}

bool FixtureService::performFixture(TasCommandModel& model, QString& message)
{
    if(!mInitialized){
        mPluginLoader->initializeFixturePlugins();
        mInitialized = true;
    }

    TasLogger::logger()->debug("FixtureService::performFixture");
    QListIterator<TasTarget*> i(model.targetList());
    message = PARSE_ERROR;
    bool result = false;
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();
        QString targetId = commandTarget->id();
        QString targetType = commandTarget->type();
        //are required for command completion
        if(targetId.isEmpty() || targetType.isEmpty()){
            continue;
        }
        void* ptr = 0;
        QString objectType = NULL_TYPE;

        if(targetType == TYPE_GRAPHICS_VIEW){
            ptr = findGraphicsItem(targetId);
            objectType = GRAPHICS_ITEM_TYPE;
        }
        else if(targetType == TYPE_STANDARD_VIEW){
            ptr = findWidget(targetId);
            objectType = WIDGET_TYPE;
        }
        else if(targetType == TYPE_WINDOW_VIEW) {
            ptr = findWindow(targetId);
            objectType = QQUICKVIEW_TYPE;
        }
        else if(targetType == TYPE_QSCENEGRAPH){
            ptr = findQuickItem(targetId);
            objectType = QQUICKITEM_TYPE;
        }
        else if(targetType == TYPE_APPLICATION_VIEW){
            ptr = qApp;
            objectType = APPLICATION_TYPE;
        }
        //only one supported for now
        TasCommand* fixture = commandTarget->findCommand("Fixture");
        if(fixture){
            QString plugin = fixture->parameter("plugin");
            QString method = fixture->parameter("method");
            TasFixturePluginInterface* fixturePlugin = mPluginLoader->loadFixtureInterface(plugin);
            if(fixturePlugin){
                TasLogger::logger()->debug("FixtureService::performFixture fixture found");
                QHash<QString,QString> parameters = fixture->getApiParameters();
                parameters.insert(OBJECT_TYPE, objectType);
                TasLogger::logger()->debug("FixtureService::performFixture fixture execute");
                QString stdOut;
                result = fixturePlugin->execute(ptr, method, parameters, stdOut);
                message = stdOut;
            }
            else{
                message = "No fixture plugin found for the given id or path: " + plugin;
            }
        }
        break;
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
        mTimer.stop();
    }
}
