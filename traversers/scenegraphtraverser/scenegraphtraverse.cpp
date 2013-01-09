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

#include <testabilityutils.h>
#include <taslogger.h>
#include "scenegraphtraverse.h"

#include <QQuickItem>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>

/*!
    \class SceneGraphTraverse
    \brief SceneGraphTraverse traverse QWidgets and QGraphicsItems

    Traverse the basic qt ui elements
*/

/*!
    Constructor
*/
SceneGraphTraverse::SceneGraphTraverse(QObject* parent)
    : QObject(parent)
{
    mTraverseUtils = new TasTraverseUtils();
}

/*!
    Destructor
*/
SceneGraphTraverse::~SceneGraphTraverse()
{
    delete mTraverseUtils;
}

void SceneGraphTraverse::beginTraverse(TasCommand* command)
{
    mTraverseUtils->createFilter(command);
}

void SceneGraphTraverse::endTraverse()
{
    mTraverseUtils->clearFilter();
}

/*!
  Traverse QGraphicsItem based objects.
 */
void SceneGraphTraverse::traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command)
{
    Q_UNUSED(objectInfo);
    Q_UNUSED(graphicsItem);
    Q_UNUSED(command);
}

/*!
  Traverse QObject based items.
*/
void SceneGraphTraverse::traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command)
{
    Q_UNUSED(command);

    QQuickItem* item = qobject_cast<QQuickItem*>(object);

    if (item) {
        QQmlContext* context = QQmlEngine::contextForObject(object);

        if (context) {
            QString name = context->nameForObject(object);
            objectInfo->addAttribute("QML_ID", name);
        }

        mTraverseUtils->addObjectDetails(objectInfo, object);

        objectInfo->addAttribute("objectType", TYPE_QSCENEGRAPH);

        QPointF point = item->mapFromScene(QPoint());

        objectInfo->addAttribute("x_absolute", -point.x());
        objectInfo->addAttribute("y_absolute", -point.y());

        // TODO already included?
        objectInfo->addAttribute("x", item->x());
        objectInfo->addAttribute("y", item->y());
        objectInfo->addAttribute("width", item->width());
        objectInfo->addAttribute("height", item->height());
    }
}




