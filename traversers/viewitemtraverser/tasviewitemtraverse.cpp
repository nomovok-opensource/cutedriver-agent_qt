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
    if(object->inherits("QAbstractItemView")){          
        QAbstractItemView* view = qobject_cast<QAbstractItemView*>(object);        
        if(view){                     
            TasLogger::logger()->debug("TasViewItemTraverse::traverseObject the object is a QAbstractItemView");            
            QAbstractItemModel* model = view->model();
          
            int role = Qt::DisplayRole;                        
            if(model) {
                TasLogger::logger()->debug("TasViewItemTraverse::traverseObject model is valid");
                if (!view->rootIndex().isValid()) {
                    TasLogger::logger()->debug("TasViewItemTraverse::traverseObject not valid");
                }
                if(view->model()->inherits("QSortFilterProxyModel")){                
                    QSortFilterProxyModel* proxy = qobject_cast<QSortFilterProxyModel*>(model);
                    if(proxy){
                        role = proxy->sortRole();
                    }
                }            
                traverseIndexLevel(view, model, role, view->rootIndex(), objectInfo);
            }
        }            
    }
}
            
void TasViewItemTraverse::traverseIndexLevel(QAbstractItemView* view, QAbstractItemModel *model, 
                                            int role, QModelIndex parent, TasObject* objectInfo)
{
    TasLogger::logger()->debug("TasViewItemTraverse::traverseIndexLevel start ");
    if(model->hasChildren(parent)){  
        TasLogger::logger()->debug("TasViewItemTraverse::traverseObject has children");
        int rows = model->rowCount(parent);
        int columns = model->columnCount(parent);
        TasLogger::logger()->debug("TasViewItemTraverse::traverseObject " + QString::number(rows) + " rows and " + 
                                   QString::number(columns));
        for(int i = 0 ; i < rows; i++){
            for(int j = 0 ; j < columns; j++){
                QModelIndex index = model->index(i, j, parent);
                if (index.isValid()) {
                    TasLogger::logger()->debug("TasViewItemTraverse::traverseIndexLevel calling fill " + 
                                               QString::number(i) + ":"+QString::number(j));
                    fillTraverseData(view, model->data(index, role), objectInfo, index);
                    TasLogger::logger()->debug("TasViewItemTraverse::traverseIndexLevel calling traverseIndexLevel");
                    traverseIndexLevel(view, model, role, index, objectInfo);
                    
                }
            }
        }        
    }   
}             
            
            
void TasViewItemTraverse::fillTraverseData(QAbstractItemView* view, QVariant data, TasObject* objectInfo, QModelIndex index)
{    
    TasLogger::logger()->debug("TasViewItemTraverse::fillTraverseData filling traverse data");
    if (!data.isValid()) {
        return;
    }

    QRect rect = view->visualRect(index );
    if(rect.isValid() && view->viewport()->visibleRegion().intersects(rect)){
        TasObject& viewItem = objectInfo->addObject();
        viewItem.setType("ItemData");
        viewItem.setName("NoName");
        //viewItem.setId((int)&viewItem);                
        viewItem.setId((quint32)&viewItem);                
        viewItem.addAttribute("row",index.row());
        viewItem.addAttribute("column",index.column());
        viewItem.addAttribute("width",rect.width());
        viewItem.addAttribute("height",rect.height());
        viewItem.addAttribute("text",data.toString()); 

        // The margin eats away the available space for the text
        // TODO check from Qt code how actually the available space is being calculated        
        addTextInfo(&viewItem, data.toString(), view->font(),
                    (rect.width() - view->contentsMargins().right() - view->contentsMargins().left()));
        
        //(rect.width() - view->frameWidth() - view->contentsMargins().left() - view->contentsMargins().right()));

        //add viewportid, needed for control
        viewItem.addAttribute("viewPort",QString::number((int)view->viewport()));

        QGraphicsProxyWidget* proxy = TestabilityUtils::parentProxy(view);

        QPoint proxyPos(0,0);
        TasLogger::logger()->debug("TasViewItemTraverse::fillTraverseData searching for proxy");        
        if (proxy) {
            // If the webview is inside a graphics proxy, 
            // Take the proxy widget global position
            proxyPos = TestabilityUtils::proxyCoordinates(proxy);
            TasLogger::logger()->debug("TasViewItemTraverse::fillTraverseData got proxy and coords" + 
                                       QString::number(proxyPos.x())+
                                       ":"+
                                       QString::number(proxyPos.y()));
            viewItem.addAttribute("objectType", "Web");
        } else {
            viewItem.addAttribute("objectType","ViewItem");                        
        }

        //print window coordinates        
        QPoint windowPoint = view->viewport()->mapTo(view->window(),rect.topLeft());



        viewItem.addAttribute("visible","true");                            
        viewItem.addAttribute("x", windowPoint.x());
        viewItem.addAttribute("y", windowPoint.y());                            
        //print screen coordinates, offset by proxy
        QPoint screenPoint = view->viewport()->mapToGlobal(rect.topLeft());
        if (proxy) {
            screenPoint = windowPoint+proxyPos;
        }
        viewItem.addAttribute("x_absolute", screenPoint.x());
        viewItem.addAttribute("y_absolute", screenPoint.y());     
    
    }
}


