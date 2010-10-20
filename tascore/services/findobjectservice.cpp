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

bool FindObjectService::addObjectDetails(TasObject& parent, TasTargetObject *targetObj, TasCommand* command)
{
    //must be set
    if(targetObj->className().isEmpty()){
        return false;
    }

    QObject* object = searchForObject(targetObj);
    if(object){
        TasObject& objectData = parent.addObject();
        bool traverseChildren = false;
        if(targetObj->child()){
            //if the child object is not found we traverse everything from object found
            if(!addObjectDetails(objectData, targetObj->child(), command)){
                traverseChildren = true;
            }
        }
        mTraverser->traverseObject(objectData, object, command, traverseChildren);
        return true;
    }
    else{
        return false;
    }
}

QObject* FindObjectService::searchForObject(TasTargetObject *targetObj)
{
    QObject* targetObject = 0;
    foreach(QWidget* widget, qApp->allWidgets()){
        //check if widget
        if(isMatch(widget, targetObj)){
            targetObject = widget;
            break;
        }

        //look from children
        //1. try finding using the object name
        if(!targetObj->objectName().isEmpty()){
            QList<QObject*> possibleTargets = widget->findChildren<QObject*>(targetObj->objectName());
            targetObject = findMatchingObject(possibleTargets, targetObj);
        }
        //2. not found, look based on class name
        if(!targetObject && !targetObj->className().isEmpty()){
            targetObject = findMatchingObject(widget->children(), targetObj);
        }
        if(!targetObject){
            //3. Maybe a graphicsView
            QGraphicsView* view = qobject_cast<QGraphicsView*>(widget);
            if(view){             
                foreach(QGraphicsItem* item, view->items()){
                    QGraphicsObject* object = item->toGraphicsObject();
                    if (object){
                        if(isMatch(object, targetObj)){
                            targetObject = object;
                            break;
                        }           
                    }
                }
            }
        }
        if(targetObject){
            break;
        }
    }

    return targetObject;
}

QObject* FindObjectService::findMatchingObject(QList<QObject*> objectList, TasTargetObject *targetObj)
{
    QObject* target = 0;    
    foreach(QObject* candidate, objectList){
        if(isMatch(candidate, targetObj)){
            target = candidate;
            break;                    
        }
    }   
    return target;
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
    if(candidate->metaObject()->className() == targetObj->className()){
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
