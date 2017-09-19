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

#include <QHash>

#include "tasconstants.h"
#include "tasqtdatamodel.h"
#include "tastraverseinterface.h"
#include "testabilityutils.h"
#include "tastraverseutils.h"

class TAS_EXPORT TasUiTraverser : public TestabilityUtils
{
public:
    TasUiTraverser(QHash<QString, TasTraverseInterface*> traversers);
    ~TasUiTraverser();

    TasDataModel* getUiState(TasCommand* command);
    TasObject& addModelRoot(TasDataModel& model, TasCommand* command=0);
    void traverseObject(TasObject& objectData, QObject* object, TasCommand* command, bool traverseChildren=true);
    void traverseGraphicsItem(TasObject& objectInfo, QGraphicsItem* graphicsItem, TasCommand* commad, bool traverseChildren=true);
    void setFilterLists(TasCommand* command);
    void initializeTraverse(TasCommand* command);
    void finalizeTraverse();

private:
    void addApplicationDetails(TasObject& application, TasCommand* command);
    void traverseGraphicsItemList(TasObject& parent, QGraphicsItem* graphicsItem, TasCommand* command);
    void traverseGraphicsViewItems(TasObject& parent, QGraphicsView* view, TasCommand* command);
    bool filterPlugin(const QString& pluginName);
    void addActions(TasObject& parentObject, QList<QAction*> actions);
    void printActions(TasObject& objectInfo, QObject* object);


private:
	  QHash<QString, TasTraverseInterface*> mTraversers;
    QStringList mPluginBlackList;
    QStringList mPluginWhiteList;
    TasTraverseUtils* mTraverseUtils;
};


#endif
