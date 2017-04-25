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


#ifndef SCENEGRAPHTRAVERSE_H
#define SCENEGRAPHTRAVERSE_H

#include <QObject>
#include <QWidget>
#include <QGraphicsItem>
#include <tastraverseutils.h>

#include "tasqtdatamodel.h"
#include "tastraverseinterface.h"


class SceneGraphTraverse : public QObject, public TasTraverseInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.nokia.testability.SceneGraphTraverse" FILE "scenegraphtraverse.json")
    Q_INTERFACES(TasTraverseInterface)

public:
    SceneGraphTraverse(QObject* parent=0);
    ~SceneGraphTraverse();

    void traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command = 0);
    void traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command = 0);
    void beginTraverse(TasCommand* command);
    void endTraverse();

private:
    TasTraverseUtils* mTraverseUtils;

    void process(TasObject* objectInfo, QObject* object, TasCommand* command);

};

#endif

