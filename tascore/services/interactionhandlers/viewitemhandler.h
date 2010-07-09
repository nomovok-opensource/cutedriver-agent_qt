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
 


#ifndef VIEWITEMHANDLER_H
#define VIEWITEMHANDLER_H

#include <QApplication>
#include <QTreeWidget>
#include <QWidget>

#include "uicommandservice.h"

class ViewItemHandler : public InteractionHandler
{
public:
    ViewItemHandler();
    ~ViewItemHandler();
  
	bool executeInteraction(TargetData data);

private:
	void setCheckState(QTreeWidget* treeWidget, TasCommand& command);
	QTreeWidgetItem* findTreeItem(QTreeWidget* treeWidget, const QString& itemId);
	QTreeWidgetItem* findFromTreeItem(QTreeWidgetItem* parent, const QString& itemId);
  
};

#endif
