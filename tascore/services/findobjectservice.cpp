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
    Q_UNUSED(response);
    if(model.service() == serviceName()){
        TasDataModel* model = new TasDataModel();
        TasObject& application = mTraverser->addModelRoot(*model);

        TasTarget* target = 0;
        foreach(target, model.targetList()){
            TasTargetObject *targetObj = target->targetObject();
            QObject* targetObject = 0;
            //1. try finding using the object name
            if(!targetObj->objectName().isEmpty()){
                QList<QObject*> possibleTargets = qApp->findChildren<QObject*>(targetObj->objectName());
                targetObject = findMatchingObject(possibleTargets, targetObj->className(), targetObj->searchParameters());
            }
            //2. not found, look based on class name
            if(!targetObject && !targetObj->className().isEmty()){
                targetObject = findMatchingObject(qApp->children, targetObj->className(), targetObj->searchParameters());
            }
        }

        return true;
    }    
    else{
        return false;
    }
}

QObject* FindObjectService::findMatchingObject(QList<QObject*> objectList, QString className, QHash<QString,QString> properties)
{
    QObject* target = 0;    
    QObject* candidate = 0;
    foreach(candidate, objectList){
        //check class name
        if(className.isEmty()){
            if(!candidate.inherits(className)){
                continue;
            }
        }
        //class name ok, check props
        if(propertiesMatch(properties, candidate)){
            target = candidate;
            break;                    
        }
    }   
    return target;
}

bool FindObjectService::propertiesMatch(QHash<QString,QString>, QObject* object)
{
    return true;
}
