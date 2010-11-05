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
TasCommand::TasCommand(QDomElement& element)
    :mElement(element)
{
}


// TasCommand::TasCommand(const TasCommand& command)
// {

//     mElement = command.documentElement();
// }

QElement& TasCommand::documentElement()
{}

/*!
    Desstructor for TasCommand.
*/
TasCommand::~TasCommand()
{
}

QElement TasCommand::documentElement()
{
    return mElement
}

/*!
    Returns the name of the command.
*/
QString TasCommand::name() const
{
    return mElement.attribute("name");
}

/*!
    Returns the textual value for the command.
*/
QString TasCommand::text() const
{
    return mElement.text();
}


/*!
    Return a parameter for the value. The QString can be empty and
    should always be verified using QString::isEmpty
*/
QString TasCommand::parameter(const QString& name)
{
    return mElement.attribute(name);
}

/*!
    Return a parameter for the value. The QString can be empty and
    should always be verified using QString::isEmpty
*/
QString TasCommand::apiParameter(const QString& name)
{
    QString value;
    QDomNodeList apiParams = target.elementsByTagName(QString("param"));
    for(int i = 0 ; i < apiParams.count(); i++){
        QDomElement apiParam = apiParams.item(i).toElement();
        if(apiParam.attribute("name") == name){
            value = apiParam.attribute("value");
            break;
        }
    }
    return value;
}

/*! 
  \class TasTargetObject
  \brief Search details about the target object or objecttree.

  TasTargetObject provides the details to search for the object 
  accessed from host side. Details used are object name, class name and
  a variety of search parameters (properties).

 */

TasTargetObject::TasTargetObject(QDomElement& element)
    :mDomElement(element)
{
    mChild = 0;
    if(!mElement.firstChildElement(QString("object")).isNull()){
        mChild = new TasTargetObject(mElement.firstChildElement(QString("object")));
    }
}

TasTargetObject::~TasTargetObject()
{
    if(mChild){
        delete mChild;
    }
}

/*!
  Object name of the object to be searched. Can be empty.
 */
QString TasTargetObject::objectName() const
{
    return mElement.attribute("objectName")
}

/*!
  Class name of the object to be searched. Must be set.
 */
QString TasTargetObject::className() const
{
    return mElement.attribute("className")
}

/*!
  Set of search parameters.
 */
QHash<QString,QString> TasTargetObject::searchParameters() const
{
    QHash<QString,QString> params;
    QDomNamedNodeMap attributes = objectDetails.attributes();
    for(int i = 0 ; i < attributes.count(); i++){
        QDomNode node = attributes.item(i);
        //strip special attrs
        if(node.nodeName() != "objectName" && node.nodeName() != "className" && node.nodeName() != "tasId"){
            params.insert(node.nodeName(), node.nodeValue());
        }
    }
    return params;
}

QString TasTargetObject::objectId() const
{
    return mElement.attribute("tasId")
}

TasTargetObject* TasTargetObject::child() const
{
    return mChild;
}

QElement& TasTargetObject::documentElement()
{
    return mElement;
}


/*!
    \class TasTarget
    \brief TasTarget is the target object for the commands e.g QWidget     
    
    TasTarget is the target object for the commands e.g QWidget
     
*/

/*!
    Constructor for TasTarget. Id required.
*/
TasTarget::TasTarget(QDomElement& element)
    :mElement(element)
{
   if(!mElement.firstChildElement(QString("object")).isNull()){
        mTargetObject = new TasTargetObject(mElement.firstChildElement(QString("object")));
   }
   QDomNodeList commands = mElement.elementsByTagName(QString("Command"));
   for(int i = 0 ; i < commands.count(); i++){
       mCommands.append(new TasCommand(commands.item(i).toElement()));
   }
}

/*!
    Destructor.
*/
TasTarget::~TasTarget()
{
    qDeleteAll(mCommands);
    mCommands.clear();
    if(mTargetObject){
        delete mTargetObject;
    }
}

TasTargetObject* TasTarget::targetObject() const
{
    return mTargetObject;
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
    Returns the id for the TasTarget.
*/
QString TasTarget::id() const 
{
    return mElement.attribute("TasId");
}

/*!
    Returns the type for the TasTarget.
*/
QString TasTarget::type() const 
{
    return mElement.attribute("type");
}

QElement& TasTarget::documentElement()
{
    return mElement;
}


/*!
    \class TasTarget
    \brief TasTarget is the target object for the commands e.g QWidget     
    
    TasTarget is the target object for the commands e.g QWidget
     
*/

/*!
    Constructor for TasCommandModel.
*/
TasCommandModel::TasCommandModel(QDomDocument* document)
{
    mDocument = document;
    mElement = mDocument->documentElement();
    QDomNodeList targets = doc.elementsByTagName (QString("Target"));
    for (int i = 0; i < targets.count(); i++){
        mTargets.append(new TasTarget(targets.item(i).toElement()));
    }
}

TasCommandModel* TasCommandModel::makeModel(const QString& sourceXml)
{
    TasCommandModel* model = 0;
    QDomDocument doc("TasCommands");    
    QString errorMsg;
    if (doc.setContent(sourceXml, &errorMsg)){
        model = new TasCommandModel(doc);
    }
    else{
        TasLogger::logger()->error("TasCommandModel::makeModel Could not parse the xml. Reason: " + errorMsg);
    }
    return model;
}

/*!
    Destructor
*/
TasCommandModel::~TasCommandModel()
{
    qDeleteAll(mTargets);
    mTargets.clear();    
    delete mDocument;
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
    Return the model id, can be null.
*/
QString TasCommandModel::id() const
{
    return mElement.attribute("id")
}

/*!
    Return the model id, can be null.
*/
QString TasCommandModel::uid() const
{
    return mElement.attribute("applicationUid")
}

/*!
    Return the model name, can be null.
*/
QString TasCommandModel::name() const
{
    return mElement.attribute("name")
}

/*!
    Return the model service, can be null.
*/
QString TasCommandModel::service() const
{
    return mElement.attribute("service")
}

int TasCommandModel::interval()
{
    return mElement.attribute("interval").toInt()
}

QString TasCommandModel::sourceString() const
{
    return mSourceString;
}

bool TasCommandModel::isAsynchronous()
{
    return if(mElement.attribute("async") == "true");
}

bool TasCommandModel::isMultitouch()
{
    return if(mElement.attribute("multitouch") == "true");

}



