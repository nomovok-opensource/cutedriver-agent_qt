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
 


#ifndef TASVIEWITEMTRAVERSE_H
#define TASVIEWITEMTRAVERSE_H

#include <QObject>
#include <QHash>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QTreeWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QHeaderView>

#include <tasqtdatamodel.h>

#include "tastraverseinterface.h"

class TasViewItemTraverse :  public QObject, public TasTraverseInterface
 {
 Q_OBJECT
 Q_INTERFACES(TasTraverseInterface)
 
 public:
     TasViewItemTraverse(QObject* parent=0);
     ~TasViewItemTraverse();

     void traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command = 0);

     void traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command = 0);     
     
private:

	 void traverseAbstractItemView(QAbstractItemView* view, TasObject* objectInfo);
	 void traverseTreeWidget(QTreeWidget* treeWidget, TasObject* objectInfo);	 
	 void traverseTableWidget(QTableWidget* tableWidget, TasObject* objectInfo);	 
	 void traverseListWidget(QListWidget* listWidget, TasObject* objectInfo);	 
	 void traverseHeaderView(QHeaderView* headerView, TasObject* objectInfo);	 

	 void traverseTreeWidgetItem(QTreeWidgetItem* item, TasObject& objectInfo, QTreeWidget *treeWidget);
	 void traverseTableWidgetItem(QTableWidgetItem* item, TasObject& objectInfo, QTableWidget *tableWidget);

	 void traverseIndexLevel(QAbstractItemView* view, QAbstractItemModel *model, int role, QModelIndex parent, TasObject* objectInfo);   
	 void fillTraverseData(QAbstractItemView* view, QVariant data, TasObject* objectInfo, QModelIndex index);
	 bool isItemVisible(QRect rect, QAbstractItemView* view);
	 bool addItemLocationDetails(TasObject& objectInfo, QRect rect, QAbstractItemView* view);
     
private:
	 QList<QModelIndex> mTraversed;
 };

#endif
 
