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
 


#ifndef TASLAYOUTTRAVERSE_H
#define TASLAYOUTTRAVERSE_H

#include <QObject>
#include <QLayoutItem>
#include <QGraphicsLayout>
#include <QGraphicsLayoutItem>

#include <tasqtdatamodel.h>
#include <tastraverseutils.h>

#include "tastraverseinterface.h"

class TasLayoutTraverse :  public QObject, public TasTraverseInterface
 {
 Q_OBJECT
 Q_INTERFACES(TasTraverseInterface)
 
public:
     TasLayoutTraverse(QObject* parent=0);
     ~TasLayoutTraverse();

     void traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command = 0);
     void traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command);     
	 void beginTraverse(TasCommand* command);
	 void endTraverse();
 
private:
	 void addLayoutItem(TasObject& objectInfo, QLayoutItem* item);
	 void addGraphicsWidgetLayout(TasObject& objectInfo, QGraphicsLayout* layout, QGraphicsItem* parent);
	 void addGraphicsLayoutItem(TasObject& objectInfo, QGraphicsLayoutItem *item, QGraphicsItem* parent);
	 QString printPolicy(QSizePolicy::Policy policy);

private:
	 TasTraverseUtils* mTraverseUtils;
 };

#endif
 
