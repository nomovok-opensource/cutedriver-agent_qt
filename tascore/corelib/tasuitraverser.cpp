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
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QWidget>
#include <QGraphicsView>
#include <QLocale>

#include "tasuitraverser.h"
#include "taslogger.h"
#include "testabilityutils.h"
#include "tastraverserloader.h"
#include "tasdeviceutils.h"


TasUiTraverser::TasUiTraverser(QHash<QString, TasTraverseInterface*> traversers)
{
    mTraversers = traversers;
}

TasUiTraverser::~TasUiTraverser()
{  
    mTraversers.clear();
    mPluginBlackList.clear();
    mPluginWhiteList.clear();
}


void TasUiTraverser::setFilterLists(TasCommand* command)
{
    mPluginBlackList.clear();
    mPluginWhiteList.clear();

    if(!command) return;
    
    if(!command->apiParameter("pluginBlackList").isEmpty()){
        mPluginBlackList = command->apiParameter("pluginBlackList").split(",");
    }

    if(!command->apiParameter("pluginWhiteList").isEmpty()){
        mPluginWhiteList = command->apiParameter("pluginWhiteList").split(",");
    }

}

bool TasUiTraverser::filterPlugin(const QString& pluginName)
{
    bool filter = true;

    if(mPluginWhiteList.isEmpty() && mPluginBlackList.isEmpty()){
        filter = false;
    }
    //black list is valued higher than white list
    else if(mPluginWhiteList.contains(pluginName) && !mPluginBlackList.contains(pluginName)){
        filter = false;
    }    
    else if(mPluginWhiteList.isEmpty() && !mPluginBlackList.contains(pluginName)){
        filter = false;
    }
    return filter;
}


void TasUiTraverser::initializeTraverse(TasCommand* command)
{
    setFilterLists(command);
    //let traversers know that a new traverse operation is starting
    QHashIterator<QString, TasTraverseInterface*> traversers(mTraversers);
    while (traversers.hasNext()) {
        traversers.next();
        traversers.value()->beginTraverse(command);
    }
}

void TasUiTraverser::finalizeTraverse()
{
    //let traversers know that a new traverse operation is starting
    QHashIterator<QString, TasTraverseInterface*> traversers(mTraversers);
    while (traversers.hasNext()) {
        traversers.next();
        traversers.value()->endTraverse();
    }
}

TasObject& TasUiTraverser::addModelRoot(TasDataModel& model, TasCommand* command)
{
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = model.addNewObjectContainer(1, qtVersion, "qt");
    
    TasObject& application = container.addNewObject(QString::number(qApp->applicationPid()), getApplicationName(), "application");          
    addApplicationDetails(application, command);
    return application;
}


TasDataModel* TasUiTraverser::getUiState(TasCommand* command)
{
    initializeTraverse(command);

    TasDataModel* model = new TasDataModel();
    TasObject& application = addModelRoot(*model, command);

    QWidgetList widgetList = qApp->topLevelWidgets();
    if (!widgetList.empty()){
        QListIterator<QWidget*> iter(qApp->topLevelWidgets());
        while(iter.hasNext()){
            QWidget *widget = iter.next();
            //only print widgets if they are visible
            if (!widget->graphicsProxyWidget() &&  (TestabilityUtils::isCustomTraverse() || widget->isVisible())){
                //widgets that have a parent will not be traversed unless the parent is the app
                //this is done to avoid objects being traversed more than once
                if(!widget->parent() || widget->parent() == qApp){
                    traverseObject(application.addObject(), widget, command);
                }
            }            
        }
    }

    finalizeTraverse();
    return model;
}
    


void TasUiTraverser::traverseObject(TasObject& objectInfo, QObject* object, TasCommand* command, bool traverseChildren)
{
    QHashIterator<QString, TasTraverseInterface*> i(mTraversers);
    while (i.hasNext()) {
        i.next();
        if(!filterPlugin(i.key())){
            i.value()->traverseObject(&objectInfo, object, command);
        }
    }
    if(traverseChildren){
        //check decendants
        //1. is graphicsview
        if(object->inherits("QGraphicsView")){ 
            traverseGraphicsViewItems(objectInfo, qobject_cast<QGraphicsView*>(object), command);
        }
        //2. is QGraphicsObject
        QGraphicsObject* graphicsObject = qobject_cast<QGraphicsObject*>(object);               
        if(graphicsObject){
            traverseGraphicsItemList(objectInfo, graphicsObject,command);
        }
        //3. Widget children
        else{
            QObjectList children = object->children();                
            if (!children.isEmpty()) {                    
                for (int i = 0; i < children.size(); ++i){                    
                    QObject *obj = children.at(i);
                    //only include widgets
                    if (obj->isWidgetType() && obj->parent() == object){
                        QWidget *widget = qobject_cast<QWidget*>(obj);
                        // TODO This (and other similar hacks) needs to be moved to plugins once OSS changes are done
                        if (TestabilityUtils::isCustomTraverse() || widget->isVisible() ) /*&& !wasTraversed(widget)*/{
                            traverseObject(objectInfo.addObject(), widget, command);
                        }
                    }
                }
            }        
        }
    }
}

void TasUiTraverser::traverseGraphicsItem(TasObject& objectInfo, QGraphicsItem* graphicsItem, TasCommand* command, bool traverseChildren)
{
    QGraphicsObject* object = graphicsItem->toGraphicsObject();
    if (object) {
        traverseObject(objectInfo, object, command);
        // Traverse the actual widget under the proxy, if available
        QGraphicsProxyWidget* proxy = qobject_cast<QGraphicsProxyWidget*>(object);
        if (proxy) {
            traverseObject(objectInfo.addObject(), proxy->widget(), command, traverseChildren);        
        }      
    }
    else{
        objectInfo.setType("QGraphicsItem");
        QHashIterator<QString, TasTraverseInterface*> i(mTraversers);
        while (i.hasNext()) {
            i.next();
            if(!filterPlugin(i.key())){
                i.value()->traverseGraphicsItem(&objectInfo, graphicsItem, command);
            }
        }
        if(traverseChildren){
            traverseGraphicsItemList(objectInfo, graphicsItem, command);
        }
    }    
}

void TasUiTraverser::traverseGraphicsItemList(TasObject& parent, QGraphicsItem* graphicsItem, TasCommand* command)
{
    foreach (QGraphicsItem* item, graphicsItem->childItems()){
        if(graphicsItem == item->parentItem()){
            // TODO This needs to be moved to plugins once OSS changes are done
            if(TestabilityUtils::isCustomTraverse()|| item->isVisible() ) {                
                traverseGraphicsItem(parent.addObject(), item, command);
            }
        }
    }        
}

void TasUiTraverser::traverseGraphicsViewItems(TasObject& parent, QGraphicsView* view, TasCommand* command)
{ 
    foreach(QGraphicsItem* item, view->items()){
        if(item->parentItem() == 0){
            // TODO This needs to be moved to plugins once OSS changes are done
            if(TestabilityUtils::isCustomTraverse() || item->isVisible()  
                ) {                
                traverseGraphicsItem(parent.addObject(), item, command);
            }
        }
    }        
}



void TasUiTraverser::addApplicationDetails(TasObject& application, TasCommand* command)
{
    traverseObject(application, qApp, command, false);

    //set these again cause properties overwrite them
    application.setName(getApplicationName());
    application.setId(QString::number(qApp->applicationPid()));
#ifdef Q_OS_SYMBIAN
    quintptr uid = CEikonEnv::Static()->EikAppUi()->Application()->AppDllUid().iUid;
    application.addAttribute("applicationUid", QString::number(uid));    
#endif    


    application.addAttribute("exepath", qApp->applicationFilePath().toLatin1().data());    
    application.addAttribute("FullName", qApp->applicationFilePath().toLatin1().data());    
    application.addAttribute("dirpath", qApp->applicationDirPath().toLatin1().data());
    application.addAttribute("processId", QString::number(qApp->applicationPid()).toLatin1().data());
    application.addAttribute("version", qApp->applicationVersion().toLatin1().data());
    application.addAttribute("objectType", TYPE_APPLICATION_VIEW);
    application.addAttribute("objectId", TasCoreUtils::objectId(qApp));

    int mem = TasDeviceUtils::currentProcessHeapSize();
    if(mem != -1){
        application.addAttribute("memUsage", mem);
    }
    QLocale defaultLocale;
    application.addAttribute("localeName", defaultLocale.name());
    application.addAttribute("localeCountry", defaultLocale.countryToString(defaultLocale.country()));
    application.addAttribute("localeLanguage", defaultLocale.languageToString(defaultLocale.language()));
}
