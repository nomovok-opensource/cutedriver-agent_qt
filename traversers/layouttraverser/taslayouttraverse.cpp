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
#include <QWidget>
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QGraphicsGridLayout>
#include <QLayout>

#include <testabilityutils.h>

#include "taslayouttraverse.h"

Q_EXPORT_PLUGIN2(layouttraverse, TasLayoutTraverse)

/*!
    \class TasLayoutTraverse
    \brief TasLayoutTraverse traverse layouts
        
    Adds a layout object to the given object if it has a layout
*/

/*!
    Constructor
*/
TasLayoutTraverse::TasLayoutTraverse(QObject* parent)
    :QObject(parent)
{
    mTraverseUtils = new TasTraverseUtils();
}

/*!
    Destructor
*/
TasLayoutTraverse::~TasLayoutTraverse()
{
    delete mTraverseUtils;
}

void TasLayoutTraverse::beginTraverse(TasCommand* command)
{
    mTraverseUtils->createFilter(command);
}

void TasLayoutTraverse::endTraverse()
{
    mTraverseUtils->clearFilter();
}

/*!
  Traverse graphicsitem(widget) for possible layout objects
*/
void TasLayoutTraverse::traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command)
{
    if (graphicsItem->isWindow() || graphicsItem->isWidget()) {
        QGraphicsWidget* widget = (QGraphicsWidget*)graphicsItem;        
        if(widget && widget->layout()){
            addGraphicsWidgetLayout(objectInfo->addObject(), widget->layout(), widget);
        }
    }
}
/*!
  Traverse object(widget) for possible layout objects
*/
void TasLayoutTraverse::traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command)
{
    if (object->isWidgetType()){
        QWidget* widget = qobject_cast<QWidget*>(object);            
        if(widget){
            QLayout* layout = widget->layout();
            if(layout){
                TasObject& layoutInfo = objectInfo->addObject();
                mTraverseUtils->addObjectDetails(&layoutInfo, layout);
                layoutInfo.addAttribute("count", layout->count());
                layoutInfo.addBooleanAttribute("enabled", layout->isEnabled());
                addLayoutItem(layoutInfo, layout);
                for(int i = 0 ; i < layout->count(); i++){
                    QLayoutItem* item = layout->itemAt(i);
                    if(item){
                        addLayoutItem(layoutInfo.addObject(), item);
                    }
                }
            }
        }
    }
    else if(object->inherits("QGraphicsWidget")){
        QGraphicsWidget* graphicsWidget = qobject_cast<QGraphicsWidget*>(object);               
        if(graphicsWidget && graphicsWidget->layout()){
            addGraphicsWidgetLayout(objectInfo->addObject(), graphicsWidget->layout(), graphicsWidget);
        }
    }
}

void TasLayoutTraverse::addLayoutItem(TasObject& objectInfo, QLayoutItem* item)
{
    if(item){
        if(!item->layout()){
            objectInfo.addAttribute("objectType", TYPE_LAYOUT_ITEM);
            objectInfo.setType("QLayoutItem");           
            objectInfo.setId(TasCoreUtils::pointerId(item));
        }
        else{
            objectInfo.addAttribute("objectType", TYPE_LAYOUT);
        }

        if(item->widget()){
            objectInfo.addAttribute("itemId", TasCoreUtils::objectId((QObject*)item->widget()));
        }

        objectInfo.addAttribute("minimumSize", item->minimumSize());
        objectInfo.addAttribute("maximumSize", item->maximumSize());    
        objectInfo.addAttribute("geometry", item->geometry());
        objectInfo.addBooleanAttribute("hasHeightForWidth", item->hasHeightForWidth());
        objectInfo.addAttribute("expandingDirections", item->expandingDirections());      
        objectInfo.addAttribute("alignment", item->alignment());      
    }
}

void TasLayoutTraverse::addGraphicsWidgetLayout(TasObject& objectInfo, QGraphicsLayout* layout, QGraphicsItem* parent)
{
    objectInfo.setType("QGraphicsLayout");        
    objectInfo.addAttribute("count", layout->count());
    objectInfo.addBooleanAttribute("activated", layout->isActivated());
    addGraphicsLayoutItem(objectInfo, layout, parent);
    for(int i = 0 ; i < layout->count(); i++){
        QGraphicsLayoutItem *item = layout->itemAt(i);
        if(item){
            addGraphicsLayoutItem(objectInfo.addObject(), item, parent);
        }
    }
}

void TasLayoutTraverse::addGraphicsLayoutItem(TasObject& objectInfo, QGraphicsLayoutItem *item, QGraphicsItem* parent)
{
    if(item){
        objectInfo.setId(TasCoreUtils::pointerId(item));
        if(item->isLayout()){
            objectInfo.addAttribute("objectType", TYPE_LAYOUT);
        }
        else{
            objectInfo.addAttribute("objectType", TYPE_LAYOUT_ITEM);
            objectInfo.setType("QGraphicsLayoutItem");        
        }
        QRectF geometry = item->geometry();

        QGraphicsView* view = TestabilityUtils::getViewForItem(parent);
        //if(view && parent && view->isTransformed()){
        if(view && !view->viewportTransform().isIdentity()){
            QTransform transform = view->viewportTransform();
            QRectF itemSceneRect = parent->sceneBoundingRect();
            QRectF itemTransformedRect = transform.mapRect(itemSceneRect);
            
            QRectF sceneRect = parent->mapRectToScene(geometry);
            QRectF transformedRect = transform.mapRect(sceneRect);
            transformedRect.moveTopLeft(transformedRect.topLeft() - itemTransformedRect.topLeft());                        
            objectInfo.addAttribute("geometry", transformedRect);            
        }
        else{
            objectInfo.addAttribute("geometry", geometry);
        }

        if(item->graphicsItem()){            
            QGraphicsWidget* qWidget = TestabilityUtils::castToGraphicsWidget(item->graphicsItem());
            if(qWidget) {
                objectInfo.addAttribute("itemId", TasCoreUtils::objectId(qWidget));
            }
            else{
                objectInfo.addAttribute("itemId", TestabilityUtils::graphicsItemId(item->graphicsItem()));
            }
        }

        objectInfo.addAttribute("minimumSize", item->minimumSize());
        objectInfo.addAttribute("maximumSize", item->maximumSize());
        objectInfo.addAttribute("preferredSize", item->preferredSize());
        objectInfo.addAttribute("preferredHeight", item->preferredHeight());
        objectInfo.addAttribute("preferredWidth", item->preferredWidth());
        
        objectInfo.addAttribute("contentsRect", item->contentsRect());
        QSizePolicy policy = item->sizePolicy();
        objectInfo.addAttribute("horizontalStretch", policy.horizontalStretch());
        objectInfo.addAttribute("verticalStretch", policy.verticalStretch());
        objectInfo.addAttribute("verticalPolicy", printPolicy(policy.verticalPolicy()));
        objectInfo.addAttribute("horizontalPolicy", printPolicy(policy.horizontalPolicy()));
        objectInfo.addAttribute("expandingDirections", policy.expandingDirections());      
    }
}


QString TasLayoutTraverse::printPolicy(QSizePolicy::Policy policy)
{
    switch(policy)
        {
        case QSizePolicy::Fixed:
            return "Fixed";
        case QSizePolicy::Minimum:
            return "Minimum";
        case QSizePolicy::Maximum:
            return "Maximum";
        case QSizePolicy::Preferred:
            return "Preferred";
        case QSizePolicy::Expanding:
            return "Expanding";
        case QSizePolicy::MinimumExpanding:
            return "MinimumExpanding";
        case QSizePolicy::Ignored:
            return "Ignored";
        default:
            return "Unknown";
        }
}
