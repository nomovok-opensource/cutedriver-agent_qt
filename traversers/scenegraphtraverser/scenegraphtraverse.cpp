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
#include <QMetaObject>

#ifdef USE_QTQML_PRIVATE_HEADERS
// needed for getting to know the exact type of a qml component
#include <private/qqmlmetatype_p.h>
#include <private/qqmldata_p.h>
#include <private/qqmlcontext_p.h>
#endif

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

#ifdef USE_QTQML_PRIVATE_HEADERS
static void addTypeInfo(QQuickItem *object, TasObject* objectInfo)
{
    QString typeName;
    QQmlType *type = QQmlMetaType::qmlType(object->metaObject());
    if (type) {
        typeName = type->qmlTypeName();
        int lastSlash = typeName.lastIndexOf(QLatin1Char('/'));
        if (lastSlash != -1)
            typeName = typeName.mid(lastSlash+1);
    } else {
        typeName = QString::fromUtf8(object->metaObject()->className());
        int marker = typeName.indexOf(QLatin1String("_QMLTYPE_"));
        if (marker != -1)
            typeName = typeName.left(marker);

        marker = typeName.indexOf(QLatin1String("_QML_"));
        if (marker != -1) {
            typeName = typeName.left(marker);
            typeName += QLatin1Char('*');
            type = QQmlMetaType::qmlType(QMetaType::type(typeName.toLatin1()));
            if (type) {
                typeName = type->qmlTypeName();
                int lastSlash = typeName.lastIndexOf(QLatin1Char('/'));
                if (lastSlash != -1)
                    typeName = typeName.mid(lastSlash+1);
            }
        }
    }

    if (!typeName.isEmpty()) {
        objectInfo->setType(typeName);
    }

    objectInfo->addAttribute("QML_NATIVE_TYPE", typeName);

    QQmlData *ddata = QQmlData::get(object, false);
    if (ddata && ddata->outerContext && !ddata->outerContext->baseUrl.isEmpty()) {
        objectInfo->addAttribute("QML_DEFINED_IN_URL", ddata->outerContext->baseUrl.toString());
        objectInfo->addAttribute("QML_DEFINED_IN_LINE", ddata->lineNumber);
        objectInfo->addAttribute("QML_DEFINED_IN_COLUMN", ddata->columnNumber);
    }
}
#endif

void SceneGraphTraverse::process(TasObject *objectInfo, QObject *object, TasCommand *command) {
    Q_UNUSED(command);
    QQmlContext* context = QQmlEngine::contextForObject(object);

    if (context) {
        QString name = context->nameForObject(object);
        objectInfo->addAttribute("QML_ID", name);
    }

    mTraverseUtils->addObjectDetails(objectInfo, object);

    objectInfo->addAttribute("objectType", TYPE_QSCENEGRAPH);


}


/*!
  Traverse QObject based items.
*/
void SceneGraphTraverse::traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command)
{
    Q_UNUSED(command);

    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    bool found = false;
    if (item) {
        found = true;
        process(objectInfo, object, command);

#ifdef USE_QTQML_PRIVATE_HEADERS
        addTypeInfo(item, objectInfo);
#endif
        QPointF point = item->mapToScene(QPoint());

        // needed for visualizer
        objectInfo->addAttribute("x", (int)point.x());
        objectInfo->addAttribute("y", (int)point.y());
        objectInfo->addAttribute("x_absolute", (int)point.x());
        objectInfo->addAttribute("y_absolute", (int)point.y());


        objectInfo->addAttribute("x_relative", item->x());
        objectInfo->addAttribute("y_relative", item->y());

        // TODO already included?
        objectInfo->addAttribute("width", item->width());
        objectInfo->addAttribute("height", item->height());
    }

    if (!found) {
        QQuickView* quickView = qobject_cast<QQuickView*>(object);
        if (quickView) {
            process(objectInfo, object, command);
            found = true;
        }
    }

    // support for QML Window.
    if (!found && QString::fromLatin1(object->metaObject()->className()).compare("QQuickView")!=0) {
        QQuickWindow* quickWindow = qobject_cast<QQuickWindow*>(object);
        if (quickWindow) {
            process(objectInfo, object, command);
            found = true;
        }
    }

}




