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
 


#ifndef TASUITRAVERSER_H
#define TASUITRAVERSER_H

#include <QList>

#include "tasconstants.h"
#include "tasqtdatamodel.h"
#include "tastraverseinterface.h"
#include "testabilityutils.h"

class TAS_EXPORT TasUiTraverser : public TestabilityUtils
{
public:  
    TasUiTraverser(QList<TasTraverseInterface*> traversers);
	~TasUiTraverser();

	TasDataModel* getUiState(TasCommand* command);
	void traverseObject(TasObject& objectData, QObject* object, TasCommand* command);
	void traverseGraphicsItem(TasObject& objectInfo, QGraphicsItem* graphicsItem, TasCommand* commad);

private:
	void addApplicationDetails(TasObject& application, TasCommand* command);
	void traverseGraphicsItemList(TasObject& parent, QGraphicsItem* graphicsItem, TasCommand* command);
	void traverseGraphicsViewItems(TasObject& parent, QGraphicsView* view, TasCommand* command);

	void setFilters(TasCommand* command);
  
private:
	TraverseFilter* mFilter;
	QList<TasTraverseInterface*> mTraversers;
};

#endif
