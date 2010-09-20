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
#include <QHashIterator>

#include "tasqtcommandmodel.h"

/*!
    \class TasCommand
    \brief TasCommand represents a single command to be sent to the application under testing.     
    

    TasCommand contains the command to be executed and parameters needed for completing it. 
    The TasCommander uses the values to send events to qApp. 
*/

/*!
    Constructor for TasCommand. Name is required.
*/
TasCommand::TasCommand(const QString& name)
{
    setName(name);
}


TasCommand::TasCommand(const TasCommand& command)
{

    setName(command.name());
    setText(command.text());
    parameters = QHash<QString, QString>(command.getParameters());
    apiParameters = QHash<QString, QPair<QString,QString> >(command.getApiParametersAndTypes());

}

/*!
    Desstructor for TasCommand.
*/
TasCommand::~TasCommand()
{
    parameters.clear();
    apiParameters.clear();
}

/*!
    Sets the name of the command.
*/
void TasCommand::setName(const QString& name)
{
    commandName = name;
}

/*!
    Returns the name of the command.
*/
QString TasCommand::name() const
{
    return commandName;
}

/*!
    Sets the textual value for the command.
*/
void TasCommand::setText(const QString& text)
{
    valueText = text;
}

/*!
    Returns the textual value for the command.
*/
QString TasCommand::text() const
{
    return valueText;
}



/*!
    Add a parameter for the command. Name is the name of the parameter and 
    value is the value.
*/
void TasCommand::addParameter(const QString& name, const QString& value)
{
    parameters.insert(name, value);
}

/*!
    Return a parameter for the value. The QString can be empty and
    should always be verified using QString::isEmpty
*/
QString TasCommand::parameter(const QString& name)
{
    QString parameter;
    if(parameters.contains(name)){
        return parameters.value(name);
    }
    return parameter;
}


/*!
    Add a parameter for the command. Name is the name of the parameter and 
    value is the value.
*/
void TasCommand::addApiParameter(const QString& name, const QString& value, const QString& type)
{
    QPair<QString,QString> pair;
    pair.second = value;
    if(type.isEmpty()){
        pair.first = QVariant::typeToName(QVariant::String);
    }
    else{
        pair.first = type;
    }
    apiParameters.insert(name, pair);
}

/*!
    Return a parameter for the value. The QString can be empty and
    should always be verified using QString::isEmpty
*/
QString TasCommand::apiParameter(const QString& name)
{
    QString parameter;
    if(apiParameters.contains(name)){
        return apiParameters.value(name).second;
    }
    return parameter;
}

QPair<QString,QString> TasCommand::apiParameterAndType(const QString& name)
{
    QPair<QString,QString> pair;
    if(apiParameters.contains(name)){
        pair = apiParameters.value(name);
    }
    return pair;
}

QHash<QString, QString> TasCommand::getParameters() const
{
    return parameters;
}


QHash<QString, QPair<QString,QString> > TasCommand::getApiParametersAndTypes() const
{
    return apiParameters;
}


QHash<QString, QString> TasCommand::getApiParameters() const
{
    QHash<QString,QString> params;
    QHashIterator<QString, QPair<QString,QString> > i(apiParameters);
    while (i.hasNext()) {
        i.next();
        params.insert(i.key(), i.value().second);
    }
    return params;
}

/*!
    \class TasTarget
    \brief TasTarget is the target object for the commands e.g QWidget     
    
    TasTarget is the target object for the commands e.g QWidget
     
*/

/*!
    Constructor for TasTarget. Id required.
*/
TasTarget::TasTarget(const QString& id)
{
    setId(id);
}

/*!
    Copy Constructor for TasTarget. 
*/
TasTarget::TasTarget(const TasTarget& target)
{
    setId(target.id());    
    setType(target.type());
    QListIterator<TasCommand*> i(target.commandList());
    while (i.hasNext()){        
        mCommands.append(new TasCommand(*i.next()));  
    }
}

/*!
    Destructor.
*/
TasTarget::~TasTarget()
{
    qDeleteAll(mCommands);
    mCommands.clear();
}

QList<TasCommand*> TasTarget::commandList() const
{
    return mCommands;
}

/*!
  Searches for a command with given name. Returns null if none found.
*/
TasCommand* TasTarget::findCommand(const QString& commandName)
{
    TasCommand* match = 0;
    QListIterator<TasCommand*> i(commandList());
    while (i.hasNext()){
        TasCommand* command = i.next();
        if(command->name() == commandName){
            match = command;
            break;
        }
    }
    return match;
}


/*!
    Adds a new command under the targer.
*/
TasCommand& TasTarget::addCommand(const QString& name)
{
    TasCommand* command = new TasCommand(name);
    mCommands.append(command);
    return *command;
}

/*!
    Set the id for the TasTarget.
*/
void TasTarget::setId(const QString& id)
{
    mTargetId = id;
}

/*!
    Set the type for the TasTarget.
*/
void TasTarget::setType(const QString& type)
{
    mTargetType = type;
}

/*!
    Returns the id for the TasTarget.
*/
QString TasTarget::id() const 
{
    return mTargetId;
}

/*!
    Returns the type for the TasTarget.
*/
QString TasTarget::type() const 
{
    return mTargetType;
}


/*!
    \class TasTarget
    \brief TasTarget is the target object for the commands e.g QWidget     
    
    TasTarget is the target object for the commands e.g QWidget
     
*/

/*!
    Constructor for TasCommandModel.
*/
TasCommandModel::TasCommandModel()
{
    mInterval = 1; //default is one
    mAsynchronous = false;
    mForceUiUpdate = false;
    mMultitouch = false;
}

/*!
    Destructor
*/
TasCommandModel::~TasCommandModel()
{
    qDeleteAll(mTargets);
    mTargets.clear();    
}    

QList<TasTarget*> TasCommandModel::targetList()
{
    return mTargets;
}

/*!
  Searches for a tasrget with given id. Returns null if not found.
*/
TasTarget* TasCommandModel::findTarget(const QString& id)
{
    TasTarget* match = 0;
    QListIterator<TasTarget*> i(targetList());
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();
        if( commandTarget->id() == id){
            match = commandTarget;
            break;
        }
    }
    return match;
}

/*!
    Adds a new target to the model and returns a reference to it.
*/
TasTarget& TasCommandModel::addTarget(const QString& id)
{
    TasTarget* target = new TasTarget(id);
    mTargets.append(target);
    return *target;
}

/*!
    Set id to the model.
*/
void TasCommandModel::setId(const QString& id)
{
    mModelId = id;
}

/*!
    Return the model id, can be null.
*/
QString TasCommandModel::id() const
{
    return mModelId;
}
/*!
    Set name to the model.
*/
void TasCommandModel::setName(const QString& name)
{
    mModelName = name;
}

/*!
    Return the model name, can be null.
*/
QString TasCommandModel::name() const
{
    return mModelName;
}

/*!
    Set service to the model.
*/
void TasCommandModel::setService(const QString& service)
{
    mModelService = service;
}

/*!
    Return the model service, can be null.
*/
QString TasCommandModel::service() const
{
    return mModelService;
}

void TasCommandModel::setInterval(int interval)
{
    mInterval = interval;
}
int TasCommandModel::interval()
{
    return mInterval;
}

void TasCommandModel::setSourceString(const QString& sourceXml)
{
    //make a copy
    mSourceString = QString(sourceXml);
}

QString TasCommandModel::sourceString() const
{
    return mSourceString;
}

void TasCommandModel::setAsynchronous(bool asynchronous)
{
    mAsynchronous = asynchronous;
}

bool TasCommandModel::isAsynchronous()
{
    return mAsynchronous;
}

void TasCommandModel::forceUiUpdate(bool force)
{
    mForceUiUpdate = force;
}

bool TasCommandModel::forceUiUpdate()
{
    return mForceUiUpdate;
}

bool TasCommandModel::isMultitouch()
{
    return mMultitouch;
}
void TasCommandModel::setMultitouch(bool multitouch)
{
    mMultitouch = multitouch;
}

