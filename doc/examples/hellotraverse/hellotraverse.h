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



#ifndef HELLO_TRAVERSE_H
#define HELLO_TRAVERSE_H

#include <QObject>

#include <tasqtdatamodel.h>

#include "tastraverseinterface.h"

class QGraphicsProxyWidget;
class TasCommand;
class QGraphicsWebView;
class TasUiTraverser;

// Traverser class is automatically loaded when placed into
// qt plugins / traversers directory
class HelloTraverse :  public QObject, public TasTraverseInterface
 {
 Q_OBJECT
 Q_INTERFACES(TasTraverseInterface)

public:
     HelloTraverse(QObject* parent=0);
     ~HelloTraverse();

     // This method is called for *ALL* QObjects in the application
     void traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command);

     // This methods is called for *ALL* QGraphicsItem objects in the application
     void traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command);

private:
 };

#endif

