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


TasUiTraverser::TasUiTraverser(QList<TasTraverseInterface*> traversers)
{
    mTraversers = traversers;
    mFilter = new TraverseFilter(); 
}

TasUiTraverser::~TasUiTraverser()
{
    
    QMutableListIterator<TasTraverseInterface*> i(mTraversers);
    while (i.hasNext()){
        delete i.next();
    }
    mTraversers.clear();
    delete mFilter;
}

void TasUiTraverser::setFilters(TasCommand* command)
{
    if(!command){
        return;
    }
    //if the command contains filtering instructions
    //set the them for all traverser plugins
    QStringList attributeBlackList;
    if(!command->apiParameter("attributeBlackList").isEmpty()){
         attributeBlackList = command->apiParameter("attributeBlackList").split(",");
    }
    QStringList attributeWhiteList;
    if(!command->apiParameter("attributeWhiteList").isEmpty()){
        attributeWhiteList = command->apiParameter("attributeWhiteList").split(",");
    }    
    QStringList pluginBlackList;
    if(!command->apiParameter("pluginBlackList").isEmpty()){
        pluginBlackList = command->apiParameter("pluginBlackList").split(",");
    }

    QStringList pluginWhiteList;
    if(!command->apiParameter("pluginWhiteList").isEmpty()){
        pluginWhiteList = command->apiParameter("pluginWhiteList").split(",");
    }

    bool filterProps = false;
    if(command->apiParameter("filterProperties") =="true"){
        filterProps = true;
    }
    mFilter->initialize(filterProps, attributeBlackList, attributeWhiteList, pluginBlackList, pluginWhiteList);
    for (int i = 0; i < mTraversers.size(); i++) {
        mTraversers.at(i)->setFilter(mFilter);
    }        

}

TasDataModel* TasUiTraverser::getUiState(TasCommand* command)
{
    setFilters(command);
    TasDataModel* model = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = model->addNewObjectContainer(1, qtVersion, "qt");

    TasObject& application = container.addNewObject(QString::number(qApp->applicationPid()), getApplicationName(), "application");          
    addApplicationDetails(application, command);

    QWidgetList widgetList = qApp->topLevelWidgets();
    if (!widgetList.empty()){
        QListIterator<QWidget*> iter(qApp->topLevelWidgets());
        while(iter.hasNext()){
            QWidget *widget = iter.next();
            //only print widgets if they are visible
            if (!widget->graphicsProxyWidget() &&   
                (TestabilityUtils::isCustomTraverse() || widget->isVisible() 
                    )) /*&& !wasTraversed(widget)*/{
                traverseObject(application.addObject(), widget, command);
            }            
        }
    }
    //reset filters after uistate made so that thet will not be used for 
    //any other tasks done by traversers
    for (int i = 0; i < mTraversers.size(); i++) {
        mTraversers.at(i)->resetFilter();
    }    
    mFilter->clear();
    return model;
}

void TasUiTraverser::traverseObject(TasObject& objectInfo, QObject* object, TasCommand* command)
{
    for (int i = 0; i < mTraversers.size(); i++) {
        if(!mFilter->filterPlugin(mTraversers.at(i)->getPluginName())){                
            mTraversers.at(i)->traverseObject(&objectInfo, object, command);
        }
    }    
    //check decendants
    //1. is graphicsview
    if(object->inherits("QGraphicsView")){ 
        traverseGraphicsViewItems(objectInfo, qobject_cast<QGraphicsView*>(object), command);
    }
    //2. is GraphicsWidget
    QGraphicsWidget* graphicsWidget = qobject_cast<QGraphicsWidget*>(object);               
    if(graphicsWidget){
        traverseGraphicsItemList(objectInfo, graphicsWidget,command);
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

void TasUiTraverser::traverseGraphicsItem(TasObject& objectInfo, QGraphicsItem* graphicsItem, TasCommand* command)
{
    if (graphicsItem->isWindow() || graphicsItem->isWidget()) {
        QObject * object = (QObject*)((QGraphicsWidget*)graphicsItem);        
        traverseObject(objectInfo, object, command);
        // Traverse the actual widget under the proxy, if available
        QGraphicsProxyWidget* proxy = qobject_cast<QGraphicsProxyWidget*>(object);
        if (proxy) {
            traverseObject(objectInfo.addObject(), proxy->widget(), command);        
        }      
    }
    else{
        objectInfo.setType("QGraphicsItem");
        for (int i = 0; i < mTraversers.size(); i++) {
            if(!mFilter->filterPlugin(mTraversers.at(i)->getPluginName())){                
                mTraversers.at(i)->traverseGraphicsItem(&objectInfo, graphicsItem, command);
            }
        }    
        traverseGraphicsItemList(objectInfo, graphicsItem, command);
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
    traverseObject(application, qApp, command);

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
