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
 

#include <QHeaderView>
#include <QtPlugin>
#include <QDebug>

#include "tasviewitemtraverse.h"
#include "taslogger.h"
#include "testabilityutils.h"

Q_EXPORT_PLUGIN2(viewitemtraverse, TasViewItemTraverse)

/*!
    \class TasViewItemTraverse
    \brief TasViewItemTraverse traverse list model items
        
    QT lists do not provide any details about items and this makes 
    it impossible to control them. This component adds needed information 
    to the lis object (items) to allow verification and control. 
*/

/*!
    Constructor
*/
TasViewItemTraverse::TasViewItemTraverse(QObject* parent)
    :QObject(parent)
{}

/*!
    Destructor
*/
TasViewItemTraverse::~TasViewItemTraverse()
{}

void TasViewItemTraverse::traverseGraphicsItem(TasObject* /*objectInfo*/, QGraphicsItem* /*graphicsItem*/, TasCommand*)
{
}

/*!
    
    Add items in QAbstractView decendants to the object data.

*/
void TasViewItemTraverse::traverseObject(TasObject* objectInfo, QObject* object, TasCommand*)
{
    TasLogger::logger()->debug("TasViewItemTraverse::traverseObject");
    if(object->inherits("QAbstractItemView")){          

        //traverse the different types of viewitems 
        //1. QTreeWidget 
        QTreeWidget* treeWidget = qobject_cast<QTreeWidget*>(object);
        if(treeWidget){
            traverseTreeWidget(treeWidget, objectInfo);
            return;
        }
        //2. QTableWidget
        QTableWidget* tableWidget = qobject_cast<QTableWidget*>(object);
        if(tableWidget){
            traverseTableWidget(tableWidget, objectInfo);
            return;
        }
        //3. QListWidget
        QListWidget* listWidget = qobject_cast<QListWidget*>(object);
        if(listWidget){
            traverseListWidget(listWidget, objectInfo);
            return;
        }
        //4. HeaderView
        QHeaderView* header = qobject_cast<QHeaderView*>(object);        
        if(header){
            traverseHeaderView(header, objectInfo);
            return;
        }
        //For the rest use the basic view traverse
        QAbstractItemView* view = qobject_cast<QAbstractItemView*>(object);        
        if(view){                     
            traverseAbstractItemView(view, objectInfo);
            return;
        }
    }
}

void TasViewItemTraverse::traverseHeaderView(QHeaderView* headerView, TasObject* objectInfo)    
{
    objectInfo->addBooleanAttribute("clickable", headerView->isClickable());
    objectInfo->addBooleanAttribute("movable", headerView->isMovable());
    objectInfo->addAttribute("offset", headerView->offset());

    if(headerView->orientation() == Qt::Horizontal){             
        objectInfo->addAttribute("orientation", "horizontal");
    }
    else{
        objectInfo->addAttribute("orientation", "vertical");
    }

    QAbstractItemModel* model = headerView->model();         
    
    if(model) {
        int count = headerView->count();
        for(int i = 0; i < count; i++){
            if(!headerView->isSectionHidden(i)){
                TasObject& headerItem = objectInfo->addObject();
                headerItem.setId((quint32)&headerItem);
                headerItem.setType("ItemData");
                headerItem.addAttribute("section", i);
                headerItem.addAttribute("text", model->headerData(i, headerView->orientation()).toString()); 
                headerItem.addAttribute("sectionPosition", headerView->sectionPosition(i));

                int size = headerView->sectionSize(i);
                int viewPortPos = headerView->sectionViewportPosition(i);
                
                QRect itemRect;

                if(headerView->orientation() == Qt::Horizontal){                
                    itemRect.setX(viewPortPos);
                    itemRect.setY(headerView->viewport()->pos().y());
                    itemRect.setWidth(size);
                    itemRect.setHeight(headerView->viewport()->height());
                }
                else{
                    itemRect.setX(headerView->viewport()->pos().x());
                    itemRect.setY(viewPortPos);
                    itemRect.setHeight(size);
                    itemRect.setWidth(headerView->viewport()->width());
                }
                addItemLocationDetails(headerItem, itemRect, headerView);
            }
        }
    }
}

void TasViewItemTraverse::traverseTableWidget(QTableWidget* tableWidget, TasObject* objectInfo)
{
    TasLogger::logger()->debug("TasViewItemTraverse::traverseTableWidget");
    int rows = tableWidget->rowCount();
    int cols = tableWidget->columnCount();
    for(int i = 0; i < rows; i++){
        for(int j = 0 ; j < cols; j++){
            QTableWidgetItem* item = tableWidget->item(i,j);
            if(item){
                traverseTableWidgetItem(item, objectInfo->addObject(), tableWidget);
            }
        }
    }
}

void TasViewItemTraverse::traverseTableWidgetItem(QTableWidgetItem* item, TasObject& objectInfo, QTableWidget *tableWidget)
{
    TasLogger::logger()->debug("TasViewItemTraverse::traverseTableWidgetItem");
    objectInfo.setId((quint32)item);
    objectInfo.setType("QTableWidgetItem");
    objectInfo.addBooleanAttribute("selected", item->isSelected());
    objectInfo.addAttribute("checkState", item->checkState());      
    objectInfo.addAttribute("row", item->row());
    objectInfo.addAttribute("column", item->column());
    objectInfo.addAttribute("text", item->text());
    objectInfo.addAttribute("statusTip", item->statusTip());
    objectInfo.addAttribute("toolTip", item->toolTip());
    objectInfo.addAttribute("whatsThis", item->whatsThis());
    objectInfo.addAttribute("parentWidget", QString::number((quint32)tableWidget));
    addFont(&objectInfo, item->font());
    QRect rect = tableWidget->visualItemRect(item);
    if(addItemLocationDetails(objectInfo, rect, tableWidget)){
        addTextInfo(&objectInfo, item->text(), item->font(),
                    (rect.width() - tableWidget->contentsMargins().right() - tableWidget->contentsMargins().left()));
    }
}

void TasViewItemTraverse::traverseListWidget(QListWidget* listWidget, TasObject* objectInfo)
{
    int count = listWidget->count();
    for(int i = 0 ;  i < count ; i++){
        QListWidgetItem* item = listWidget->item(i);
        if(item && !item->isHidden()){
            TasObject& listItem = objectInfo->addObject();
            listItem.setId((quint32)item);
            listItem.setType("QListWidgetItem");
            listItem.addAttribute("text", item->text());
            listItem.addAttribute("textAlignment", item->textAlignment());
            listItem.addAttribute("toolTip", item->toolTip());
            listItem.addAttribute("whatsThis", item->whatsThis());
            listItem.addBooleanAttribute("selected", item->isSelected());
            listItem.addAttribute("checkState", item->checkState());
            listItem.addAttribute("parentWidget", QString::number((quint32)listWidget));
            addFont(&listItem, item->font());
            QRect rect = listWidget->visualItemRect(item);
            if(addItemLocationDetails(listItem, rect, listWidget)){
                addTextInfo(&listItem, item->text(), item->font(),
                            (rect.width() - listWidget->contentsMargins().right() - listWidget->contentsMargins().left()));
            }       
        }
    }
}

void TasViewItemTraverse::traverseTreeWidget(QTreeWidget* treeWidget, TasObject* objectInfo)
{
    //The widget is already traversed only the items are needed
    int count = treeWidget->topLevelItemCount(); 
    for(int i = 0; i < count; i++){
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item){
            traverseTreeWidgetItem(item, objectInfo->addObject(), treeWidget);
        }
    }
}

void TasViewItemTraverse::traverseTreeWidgetItem(QTreeWidgetItem* item, TasObject& objectInfo, QTreeWidget *treeWidget)
{
    objectInfo.setId((quint32)item);
    objectInfo.setType("QTreeWidgetItem");
    objectInfo.addBooleanAttribute("disabled",item->isDisabled());
    objectInfo.addBooleanAttribute("expanded", item->isExpanded());
    objectInfo.addBooleanAttribute("firstColumnSpanned", item->isFirstColumnSpanned());
    objectInfo.addBooleanAttribute("hidden", item->isHidden());
    objectInfo.addBooleanAttribute("selected", item->isSelected());
    objectInfo.addAttribute("parentWidget", QString::number((quint32)treeWidget));

    //add location
    QRect rect = treeWidget->visualItemRect(item);
    addItemLocationDetails(objectInfo, rect, treeWidget);

    //check columns 
    //will be added as object to make things more clear (no size, location details)
    int count = item->columnCount();
    for(int i = 0; i < count; i++){
        TasObject& column = objectInfo.addObject();    
        column.setId((quint32)&column);
        column.setType("TreeWidgetItemColumn");
        column.addAttribute("column", i);
        //text 
        column.addAttribute("text", item->text(i));
        addFont(&column, item->font(i));
        //toolTip
        column.addAttribute("toolTip", item->toolTip(i));
        column.addAttribute("checkState", item->checkState(i));
        column.addAttribute("whatsThis", item->whatsThis(i));
        column.addAttribute("statusTip", item->statusTip(i));
        if(item->sizeHint(i).isValid()){
            column.addAttribute("sizeHint", item->sizeHint(i));
        }
        column.addAttribute("parentWidget", QString::number((quint32)treeWidget));
        column.addAttribute("parentItem", QString::number((quint32)item));
    }
    //iterate children
    count = item->childCount();
    for(int i = 0 ; i < count; i++){
        traverseTreeWidgetItem(item->child(i), objectInfo.addObject(), treeWidget);
    }
}

void TasViewItemTraverse::traverseAbstractItemView(QAbstractItemView* view, TasObject* objectInfo)
{
    QAbstractItemModel* model = view->model();         
    int role = Qt::DisplayRole;                        
    if(model) {
        if (!view->rootIndex().isValid()) {
        }
        if(view->model()->inherits("QSortFilterProxyModel")){                
            QSortFilterProxyModel* proxy = qobject_cast<QSortFilterProxyModel*>(model);
            if(proxy){
                role = proxy->sortRole();
            }
        }    
        mTraversed.clear();        
        traverseIndexLevel(view, model, role, view->rootIndex(), objectInfo);
    }
}
            
void TasViewItemTraverse::traverseIndexLevel(QAbstractItemView* view, QAbstractItemModel *model, 
                                            int role, QModelIndex parent, TasObject* objectInfo)
{
    if(model->hasChildren(parent)){  
        int rows = model->rowCount(parent);
        int columns = model->columnCount(parent);
        for(int i = 0 ; i < rows; i++){
            for(int j = 0 ; j < columns; j++){
                QModelIndex index = model->index(i, j, parent);
                if (index.isValid() && !mTraversed.contains(index)) {
                    mTraversed.append(index);            
                    fillTraverseData(view, model->data(index, role), objectInfo, index);
                    traverseIndexLevel(view, model, role, index, objectInfo);
                }
            }
        }        
    }       
}             
            
            
void TasViewItemTraverse::fillTraverseData(QAbstractItemView* view, QVariant data, TasObject* objectInfo, QModelIndex index)
{    
    if (!data.isValid()) {
        return;
    }
    QRect rect = view->visualRect(index );
    if(isItemVisible(rect, view)){
        TasObject& viewItem = objectInfo->addObject();
        viewItem.setId((quint32)&viewItem);                
        viewItem.setType("ItemData");
        viewItem.addAttribute("row",index.row());
        viewItem.addAttribute("column",index.column());
        viewItem.addAttribute("text",data.toString()); 
        addFont(&viewItem, view->font());
        if(addItemLocationDetails(viewItem, rect, view)){   
            // The margin eats away the available space for the text
            // TODO check from Qt code how actually the available space is being calculated        
            addTextInfo(&viewItem, data.toString(), view->font(),
                        (rect.width() - view->contentsMargins().right() - view->contentsMargins().left()));
        
        }
    }
}

bool TasViewItemTraverse::isItemVisible(QRect rect, QAbstractItemView* view)
{
    bool visible = false;
    if(rect.isValid() && view->viewport()->visibleRegion().intersects(rect)){
        visible = true;
    }
    return visible;
}

/*!
  Adds location and size details to the item. 
 */
bool TasViewItemTraverse::addItemLocationDetails(TasObject& objectInfo, QRect rect, QAbstractItemView* view)
{
    
    if(isItemVisible(rect, view)){
        objectInfo.addAttribute("width",rect.width());
        objectInfo.addAttribute("height",rect.height());
        //add viewportid, needed for control
        objectInfo.addAttribute("viewPort",QString::number((int)view->viewport()));
        QGraphicsProxyWidget* proxy = TestabilityUtils::parentProxy(view);
        QPoint proxyPos(0,0);
        if (proxy) {
            // If the webview is inside a graphics proxy, 
            // Take the proxy widget global position
            proxyPos = TestabilityUtils::proxyCoordinates(proxy);
            objectInfo.addAttribute("objectType", "Web");
        } else {
            objectInfo.addAttribute("objectType","ViewItem");                        
        }

        //print window coordinates        
        QPoint windowPoint = view->viewport()->mapTo(view->window(),rect.topLeft());
        objectInfo.addAttribute("visible","true");                            
        objectInfo.addAttribute("x", windowPoint.x());
        objectInfo.addAttribute("y", windowPoint.y());                            
        //print screen coordinates, offset by proxy
        QPoint screenPoint = view->viewport()->mapToGlobal(rect.topLeft());
        if (proxy) {
            screenPoint = windowPoint+proxyPos;
        }
        objectInfo.addAttribute("x_absolute", screenPoint.x());
        objectInfo.addAttribute("y_absolute", screenPoint.y());     
        return true;
    }
    else{
        objectInfo.addAttribute("visible","false");                            
        return false;
    }
}

