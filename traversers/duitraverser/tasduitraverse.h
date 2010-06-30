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
 


#ifndef TASDUITRAVERSE_H
#define TASDUITRAVERSE_H

#include <QObject>
#include <QHash>
#include <tasqtdatamodel.h>
#include <tastraverseinterface.h>

class TasDuiTraverse :  public QObject, public TasTraverseInterface
 {
 Q_OBJECT
 Q_INTERFACES(TasTraverseInterface)
 
 public:
     TasDuiTraverse(QObject* parent=0);
     ~TasDuiTraverse();

     void traverseObject(TasObject* objectInfo, QObject* object, TasCommand*);
	 
     void traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand*);     
   
};

#endif
 
