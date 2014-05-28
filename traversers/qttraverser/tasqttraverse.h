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




#ifndef TASQTTRAVERSE_H
#define TASQTTRAVERSE_H

#include <QObject>
#include <QWidget>
#include <QGraphicsItem>
#include <tastraverseutils.h>

#include "tasqtdatamodel.h"
#include "tastraverseinterface.h"


class TasQtTraverse : public QObject, public TasTraverseInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.nokia.testability.TasQtTraverse" FILE "tasqttraverse.json")
    Q_INTERFACES(TasTraverseInterface)

public:
    TasQtTraverse(QObject* parent=0);
    ~TasQtTraverse();

    void traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command = 0);
    void traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command = 0);
    void beginTraverse(TasCommand* command);
    void endTraverse();

private:
    int getParentId(QObject* object);
    void printProperties(TasObject* objectData, QObject* object);
    void addWidgetCoordinates(TasObject* objectInfo, QWidget* widget, TasCommand* command);
    void printGraphicsItemProperties(TasObject* objectData, QGraphicsItem* graphicsItem);

private:
    TasTraverseUtils* mTraverseUtils;

};

#endif

