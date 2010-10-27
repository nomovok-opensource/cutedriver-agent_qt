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

#include <QObject>

#if QT_VERSION >= 0x040700
#include <QDeclarativeItem>
#endif


#include "taslogger.h"

#include "findobjectservice.h"
#include "tastraverserloader.h"


FindObjectService::FindObjectService()
{
    TasTraverserLoader loader;
    mTraverser = new TasUiTraverser(loader.loadTraversers());
}

FindObjectService::~FindObjectService()
{
    delete mTraverser;
}

/*!
  Passes service directed to plugins on to the correct plugin.
 */
bool FindObjectService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName()){
        //        TasLogger::logger()->debug("FindObjectService::executeService");
        bool traverseAll = false;
        TasDataModel* uiModel = new TasDataModel();
        TasObject& application = mTraverser->addModelRoot(*uiModel);

        foreach(TasTarget* target, model.targetList()){
            TasTargetObject *targetObj = target->targetObject();
            //app always returned so skip it
            if(targetObj->className().isEmpty() && targetObj->objectName().isEmpty() && 
               targetObj->searchParameters().contains("type") && targetObj->searchParameters().value("type") == "application"){
                continue;
            }
            TasCommand* command = 0;
            if(!target->commandList().isEmpty()){
                command = target->commandList().at(0);
            }
            mTraverser->initializeTraverse(command);
            if(!addObjectDetails(application, targetObj, command)){
                //not found so the we need to traverse the old way
                traverseAll = true;   
                break;
            }
            mTraverser->finalizeTraverse();
        }
        if(traverseAll){
            TasLogger::logger()->debug("FindObjectService::executeService nothing found traverse all");
            delete uiModel;
            //            TasLogger::logger()->debug("FindObjectService::executeService old model cleaned");
            TasCommand* command = 0;
            if (model.targetList().size() > 0) {
                if(!model.targetList().at(0)->commandList().isEmpty()){
                    command = model.targetList().at(0)->commandList().at(0);
                }
            }
            //            TasLogger::logger()->debug("FindObjectService::executeService normal traverse");
            uiModel = mTraverser->getUiState(command);
        }
        //        TasLogger::logger()->debug("FindObjectService::executeService make return message.");
        QByteArray* xml = new QByteArray();         
        uiModel->serializeModel(*xml);
        delete uiModel;
        response.setData(xml);
        return true;
    }    
    else{
        return false;
    }
}

bool FindObjectService::addObjectDetails(TasObject& parent, TasTargetObject *targetObj, TasCommand* command, QObject* parentObject)
{
    //must be set
    if(targetObj->className().isEmpty()){
        return false;
    }
    QList<QObject*> objects;
    //first level look from app 
    if(!parentObject){
        objects = searchForObject(targetObj);
    }
    //look children of parent
    else{
        objects = findMatchingObject(parentObject->children(), targetObj);
    }
    if(!objects.isEmpty()){
        foreach(QObject* object, objects){
            TasObject& objectData = parent.addObject();
            bool traverseChildren = false;
            if(targetObj->child()){
                //if the child object is not found we traverse everything from object found
                if(!addObjectDetails(objectData, targetObj->child(), command, object)){
                    traverseChildren = true;
                }
            }
            mTraverser->traverseObject(objectData, object, command, traverseChildren);
        }
        return true;
    }
    else{
        return false;
    }
}

QList<QObject*> FindObjectService::searchForObject(TasTargetObject *targetObj)
{
    QList<QObject*> targetObjects;
    foreach(QWidget* widget, qApp->allWidgets()){
        if(targetObjects.contains(widget) || !widget->isVisible()){
            continue;
        }
        //1. check if widget matches
        if(isMatch(widget, targetObj)){
            targetObjects.append(widget);
        }  
      
        //2.look from children
        if(targetObjects.isEmpty() && !targetObj->className().isEmpty()){
            targetObjects.append(findMatchingObject(widget->children(), targetObj));
        }

        //3. Maybe a graphicsView
        QGraphicsView* view = qobject_cast<QGraphicsView*>(widget);
        if(view){             
            foreach(QGraphicsItem* item, view->items()){
                QGraphicsObject* object = item->toGraphicsObject();
                if (object && object->isVisible() && TestabilityUtils::isItemInView(view, item)){
                    if(isMatch(object, targetObj)){
                        targetObjects.append(object);
                    }           
                }
            }        
        }
    }
    return targetObjects;
}

QList<QObject*> FindObjectService::findMatchingObject(QList<QObject*> objectList, TasTargetObject *targetObj)
{
    QList<QObject*> targets ;    
    foreach(QObject* candidate, objectList){
        QVariant visibility = candidate->property("visible");
        //skip if property visible false
        if(visibility.isValid() && visibility.type() == QVariant::Bool && visibility.toBool() == false){
            continue;
        }
        if(isMatch(candidate, targetObj)){
            targets.append(candidate);
        }
    }   
    return targets;
}

bool FindObjectService::isMatch(QObject* candidate, TasTargetObject *targetObj)
{
    //    TasLogger::logger()->debug("FindObjectService::isMatch className:" + className + " <> " + QString(candidate->metaObject()->className()));
    //check id first if given, if not same then not match
    if(!targetObj->objectId().isEmpty()){
         return targetObj->objectId() == TasCoreUtils::objectId(candidate);
    }
    //check name if given, if not same then not match
    if(!targetObj->objectName().isEmpty()){
        return targetObj->objectName() == candidate->objectName();
    }

    bool isMatch = false;

    //traverser strips _QML so need to strip it here also
    QString className = candidate->metaObject()->className();

#if QT_VERSION >= 0x040700
    if(qobject_cast<QDeclarativeItem*>(candidate)){
        className = className.split("_QML").first();
    }
#endif

    if(className == targetObj->className()){
        //class name ok, check props
        if(propertiesMatch(targetObj->searchParameters(), candidate)){
            isMatch = true;
        }
    }
    return isMatch;
}

bool FindObjectService::propertiesMatch(QHash<QString,QString> props, QObject* object)
{
    bool match = true;
    QHashIterator<QString,QString> i(props);
    while(i.hasNext()){
        i.next();
        QVariant property = object->property(i.key().toAscii());
        if(!property.isValid() || property.toString() != i.value()){
            match = false;
            break;
        }
    }
    return match;
}
