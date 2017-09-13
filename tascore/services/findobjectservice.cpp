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
#include <QObject>
#include <QQuickWindow>
#include <QQuickItem>

#include "taslogger.h"
#include "taspointercache.h"
#include "findobjectservice.h"
#include "tastraverserloader.h"


FindObjectService::FindObjectService()
{
    TasTraverserLoader loader;
    mTraverseUtils = new TasTraverseUtils();
    mTraverser = new TasUiTraverser(loader.loadTraversers());
}

FindObjectService::~FindObjectService()
{
    delete mTraverser;
    delete mTraverseUtils;
}

/*!
  Passes service directed to plugins on to the correct plugin.
 */
bool FindObjectService::executeService(TasCommandModel& model, TasResponse& response)
{
    #ifdef DEBUG_ENABLED
    TasLogger::logger()->debug(QString(">> %0").arg(Q_FUNC_INFO));
    #endif
    if(model.service() == serviceName()){
        //remove objects that are no longer valid
        TasPointerCache::instance()->removeNulls();

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
            //the request is for vkb app so return only app details
            if(targetObj->className() == VKB_IDENTIFIER && !TasCoreUtils::getApplicationName().contains(PENINPUT_SERVER)){
                traverseAll = false;
                break;
            }

            TasCommand* command = Q_NULLPTR;
            if(!target->commandList().isEmpty()){
                command = target->commandList().at(0);
            }

            mTraverser->initializeTraverse(command);
            #ifdef DEBUG_ENABLED
            TasLogger::logger()->debug(QString("! %0 %1").arg(Q_FUNC_INFO).arg("command: " + command->name() + " text: " + command->text()));
            #endif
            if (targetObj->className() == "*") {
                traverseAll = true;
                break;
            } else {
                if(!addObjectDetails(application, targetObj, command)){
                    //not found so the we need to traverse the old way
                    traverseAll = true;
                    break;
                }
            }
            mTraverser->finalizeTraverse();
        }
        if(traverseAll){
            #ifdef DEBUG_ENABLED
            TasLogger::logger()->debug(QString("! %0 %1").arg(Q_FUNC_INFO).arg("traverse all"));
            #endif
            delete uiModel;
            TasCommand* command = Q_NULLPTR;
            if (model.targetList().size() > 0) {
                if(!model.targetList().at(0)->commandList().isEmpty()){
                    command = model.targetList().at(0)->commandList().at(0);
                }
            }
            uiModel = mTraverser->getUiState(command);
        }
        QByteArray xml;
        uiModel->serializeModel(xml, model.onlyFragment());
        delete uiModel;
        response.setData(xml);
        #ifdef DEBUG_ENABLED
        TasLogger::logger()->debug(xml);
        TasLogger::logger()->debug(QString("(%1) << %0 %2").arg(Q_FUNC_INFO).arg(true).arg(xml.length()));
        #endif
        return true;
    } else {
        #ifdef DEBUG_ENABLED
        TasLogger::logger()->debug(QString("(%1) << %0 ").arg(Q_FUNC_INFO).arg(false));
        #endif
        return false;
    }
}

bool FindObjectService::addObjectDetails(TasObject& parent, TasTargetObject *targetObj, TasCommand* command, QObject* parentObject)
{
    #ifdef DEBUG_ENABLED
    TasLogger::logger()->debug(QString(">> %0 %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
    #endif
    //special check skip if vkb
    if(targetObj->className() == VKB_IDENTIFIER && TasCoreUtils::getApplicationName().contains(PENINPUT_SERVER)){
        if(targetObj->child()){
            //add the actual objects under the vkb (which really an app)
            bool retval = addObjectDetails(parent, targetObj->child(), command, parentObject);
            #ifdef DEBUG_ENABLED
            TasLogger::logger()->debug(QString("(%1) << %0 targetObj.child()").arg(Q_FUNC_INFO).arg(retval));
            #endif
            return retval;
        }
        else{
            #ifdef DEBUG_ENABLED
            TasLogger::logger()->debug(QString("(%1) << %0 !targetObj.child()").arg(Q_FUNC_INFO).arg(true));
            #endif
            //nothing needed as vkb added as app
            return true;
        }
    }


    bool cached = false;
    QList<QObject*> objects;

    //look from cache
    if (!targetObj->objectId().isEmpty()) {
        QObject* o = TasPointerCache::instance()->getObject(targetObj->objectId());
        if (o) {
            objects.append(o);
            cached = true;
        }
    }
    #ifdef DEBUG_ENABLED
    TasLogger::logger()->debug(QString("! %0 was found in cache: %1 %2").arg(Q_FUNC_INFO).arg(cached).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
    #endif
    if (!cached) {
        //first level look from app
        if (!parentObject) {
            #ifdef DEBUG_ENABLED
            TasLogger::logger()->debug(QString("! %0 !parentObject: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
            #endif

            //look from parents object list if name set
            objects = searchForObject(targetObj);
        }
        //look children of parent
        else {
            #ifdef DEBUG_ENABLED
            TasLogger::logger()->debug(QString("! %0 parentObject: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId() + " parentObject:"+parentObject->objectName()));
            #endif
            if (!targetObj->objectName().isEmpty()) {
                #ifdef DEBUG_ENABLED
                TasLogger::logger()->debug(QString("! %0 parentObject: targetObj.objectName: findobjects: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId() + " parentObject:"+parentObject->objectName()));
                #endif

                objects = parentObject->findChildren<QObject*>(targetObj->objectName());
                if (objects.isEmpty()) {
                    #ifdef DEBUG_ENABLED
                    TasLogger::logger()->debug(QString("! %0 parentObject: targetObj.objectName: objects not found: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId() + " parentObject:"+parentObject->objectName()));
                    #endif

                    QQuickItem* parentItem = qobject_cast<QQuickItem*>(parentObject);
                    if (parentItem) {
                        #ifdef DEBUG_ENABLED
                        TasLogger::logger()->debug(QString("! %0 parentObject: is QQuickItem %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId() + " parentObject:"+parentObject->objectName()));
                        #endif
                        QQuickItem* item = TestabilityUtils::findQuickItemByObjectName(parentItem, targetObj->objectName(), targetObj->objectId());
                        if (item) {
                            #ifdef DEBUG_ENABLED
                            TasLogger::logger()->debug(QString("! %0 parentObject: targetObj.objectName: found item %2 - %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId() + " parentObject:"+parentObject->objectName()).arg(item->objectName()));
                            #endif
                            objects.append(item);
                        }
                    } else {
                        #ifdef DEBUG_ENABLED
                        TasLogger::logger()->debug(QString("! %0 parentObject: targetObj.objectName: parentItem not QQuickItem: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId() + " parentObject:"+parentObject->objectName()));
                        #endif
                    }
                }

            } else if (parentObject && targetObj->className().compare("*")==0) {
                #ifdef DEBUG_ENABLED
                TasLogger::logger()->debug(QString("! %0 parentObject: and className is '*': %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                #endif
                objects.append(parentObject);
            }

            if (objects.isEmpty() && targetObj->className().compare("*")!=0) {
                #ifdef DEBUG_ENABLED
                TasLogger::logger()->debug(QString("! %0 parentObject: objects still empty: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                #endif

                objects = findMatchingObject(parentObject->children(), targetObj);

                if (objects.isEmpty()) {
                    #ifdef DEBUG_ENABLED
                    TasLogger::logger()->debug(QString("! %0 parentObject: objects still empty (findMatchingObject): %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                    #endif

                    // Quick stuff
                    objects = searchForObject(targetObj);
                    if (objects.isEmpty()) {
                        #ifdef DEBUG_ENABLED
                        TasLogger::logger()->debug(QString("! %0 parentObject: objects still empty (searchForObject): %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                        #endif
                    }
                }
            }
        }
    }

    if (!objects.isEmpty()) {
        #ifdef DEBUG_ENABLED
        TasLogger::logger()->debug(QString("! %0 objects empty: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
        #endif

        foreach (QObject* object, objects) {
            bool isFindChild = targetObj->className().compare("*")==0;
            bool traverseChildren = false;
            if (!isFindChild && targetObj->child()) {
                #ifdef DEBUG_ENABLED
                TasLogger::logger()->debug(QString("! %0 objects empty: !isFindChild and has child: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                #endif
                //if the child object is not found we traverse everything from object found
                TasObject& objectData = parent.addObject();
                if (!addObjectDetails(objectData, targetObj->child(), command, object)) {
                    #ifdef DEBUG_ENABLED
                    TasLogger::logger()->debug(QString("! %0 objects empty: !isFindChild and has child: not addObjectDetails %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                    #endif

                    traverseChildren = true;
                }
                #ifdef DEBUG_ENABLED
                TasLogger::logger()->debug(QString("! %0 objects empty: !isFindChild and has child: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                #endif
                mTraverser->traverseObject(objectData, object, command, traverseChildren);
                continue;
            }

            if (isFindChild) {
                #ifdef DEBUG_ENABLED
                TasLogger::logger()->debug(QString("! %0 objects empty: isFindChild: traverse %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                #endif
                mTraverser->traverseObject(parent, object, command, true);
            } else {
                #ifdef DEBUG_ENABLED
                TasLogger::logger()->debug(QString("! %0 objects empty: !isFindChild: traverse %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
                #endif

                TasObject& objectData = parent.addObject();
                mTraverser->traverseObject(objectData, object, command, traverseChildren);
            }
        }
        #ifdef DEBUG_ENABLED
        TasLogger::logger()->debug(QString("(%1) << %0 %2").arg(Q_FUNC_INFO).arg(true).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
        #endif
        return true;
    } else {
        #ifdef DEBUG_ENABLED
        TasLogger::logger()->debug(QString("(%1) << %0 %2").arg(Q_FUNC_INFO).arg(false).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
        #endif
        return false;
    }
}

QList<QObject*> FindObjectService::findAllObjects(QObject* parent) {
#ifdef DEBUG_ENABLED
    TasLogger::logger()->debug(QString("! %0 parent: %1").arg(Q_FUNC_INFO).arg("objectName:" + parent->objectName() + " classname:" + parent->metaObject()->className()));
#endif
    QList<QObject*> retval;

    QWindow* window = qobject_cast<QWindow*>(parent);
    if (window) {
        foreach (QObject* child, window->children()) {
            retval.append(child);
            retval.append(findAllObjects(child));
        }
        return retval;
    }

    QQuickWindow* quickWindow = qobject_cast<QQuickWindow*>(parent);
    if (quickWindow) {
        foreach (QObject* child, quickWindow->children()) {
            retval.append(child);
            retval.append(findAllObjects(child));
        }
        return retval;
    }

    QQuickItem* item = qobject_cast<QQuickItem*>(parent);
    if (item) {
        foreach (QQuickItem* child, item->childItems()) {
            retval.append(child);
            retval.append(findAllObjects(child));
        }
        return retval;
    }

    foreach (QObject* child, parent->children()) {
        retval.append(child);
        retval.append(findAllObjects(child));
    }


    return retval;
}

QList<QObject*> FindObjectService::searchForObject(TasTargetObject *targetObj)
{
#ifdef DEBUG_ENABLED
    TasLogger::logger()->debug(QString("! %0 objects empty: %1").arg(Q_FUNC_INFO).arg("objectName:" + targetObj->objectName() + " classname:" + targetObj->className() + " id:" + targetObj->objectId()));
#endif
    QList<QObject*> targetObjects;
    QString objectName = targetObj->objectName();

    foreach (QWindow *w, QApplication::allWindows()) {
        if (targetObj->className() == "*") {
            targetObjects.append(findAllObjects(w));
            break;
        }
        if (isMatch(w, targetObj)) {
            targetObjects.append(w);
            break;
        }

        QList<QObject*> found;
        if (!objectName.isEmpty()) {
            found = w->findChildren<QObject*>(objectName, Qt::FindChildrenRecursively);
            if (!found.isEmpty()) {
                targetObjects.append(found);
                continue;
            }
        }

        QQuickWindow* quickWindow = qobject_cast<QQuickWindow*>(w);
        if (quickWindow) {
            QQuickItem* root = quickWindow->contentItem();
            if (isMatch(root, targetObj)) {
                found.append(root);
                break;
            }
            if (!objectName.isEmpty()) {
                found = root->findChildren<QObject*>(objectName, Qt::FindChildrenRecursively);
            }
            if (!found.isEmpty()) {
                targetObjects.append(found);
                continue;
            } else {
                bool wasMatch = false;
                foreach(QQuickItem* childItem, root->childItems()) {
                    QQuickItem* item = TestabilityUtils::findQuickItemByObjectName(childItem, targetObj->objectName(), targetObj->objectId());
                    if (item && isMatch(item, targetObj)) {
                        targetObjects.append(item);
                        wasMatch = true;
                        break;
                    }
                }
                if (wasMatch) {
                    break;
                }
            }
        }
    }

    if (targetObjects.isEmpty()) {

        foreach(QWidget* widget, QApplication::allWidgets()){
            if(targetObjects.contains(widget) || !widget->isVisible()){
                continue;
            }
            //1. check if widget matches
            if(isMatch(widget, targetObj)){
                targetObjects.append(widget);
            }

            //2.look from children
            if(targetObjects.isEmpty()){
                if(!targetObj->objectName().isEmpty()){
                    targetObjects.append(widget->findChildren<QObject*>(targetObj->objectName()));
                }
                else{
                    targetObjects.append(findMatchingObject(widget->children(), targetObj));
                }
            }
            //3. Maybe a graphicsView
            if(targetObjects.isEmpty()){
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
        }
    }
    return targetObjects;
}

QList<QObject*> FindObjectService::findMatchingObject(QList<QObject*> objectList, TasTargetObject *targetObj)
{
    QList<QObject*> targets;
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
    #ifdef DEBUG_ENABLED
    TasLogger::logger()->debug(QString(">> %0 %1").arg(Q_FUNC_INFO).arg("objectName:" + candidate->objectName() + " classname:"+ candidate->metaObject()->className() + "\t TARGET => objectName:" + targetObj->objectName() + " id:" + targetObj->objectId() + " classname:" + targetObj->className()));
    #endif

    if (!candidate || !targetObj) return false;

    bool retval = TestabilityUtils::compareObjectName(candidate->objectName(),targetObj->objectName());

    if (!retval) {
        retval = candidate->metaObject()->className() == targetObj->className();
    }

    //check id first if given, if not same then not match
    if(!retval && !targetObj->objectId().isEmpty()){
         retval = targetObj->objectId() == TasCoreUtils::objectId(candidate);
    }

    //check name if given, if not same then not match
    if(!retval && !targetObj->objectName().isEmpty()){
        retval = targetObj->objectName() == candidate->objectName();
    }

    if (retval) {
        //traverser strips _QML so need to strip it here also
        QString className = candidate->metaObject()->className();

        if(!targetObj->className().isEmpty() && (className != targetObj->className())){
            retval = false;
        }
    }

    //all other checks ok, check props
    if (retval) {
        retval = propertiesMatch(targetObj->searchParameters(), candidate);
    }

    #ifdef DEBUG_ENABLED
    TasLogger::logger()->debug(QString("%2 << %0 %1").arg(Q_FUNC_INFO).arg("objectName:" + candidate->objectName() + " classname:"+ candidate->metaObject()->className() + "\t TARGET => objectName:" + targetObj->objectName() + " id:" + targetObj->objectId() + " classname:" + targetObj->className()).arg(retval));
    #endif
    return retval;
}

bool FindObjectService::propertiesMatch(QHash<QString,QString> props, QObject* object)
{
    bool match = true;
    QHashIterator<QString,QString> i(props);
    while(i.hasNext()){
        i.next();
        QVariant property = object->property(i.key().toLatin1());
        if(!property.isValid() || property.toString() != i.value()){
            match = false;
            break;
        }
    }
    return match;
}
