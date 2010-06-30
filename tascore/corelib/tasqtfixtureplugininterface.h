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
 


#ifndef TASQTFIXTUREPLUGININTERFACE_H
#define TASQTFIXTUREPLUGININTERFACE_H

#include <QObject>
#include <QString>
#include <QHash>

static const QString OBJECT_TYPE = "__fixture_object_type";
static const QString WIDGET_TYPE = "QWidget";
static const QString GRAPHICS_ITEM_TYPE = "QGraphicsItem";
static const QString APPLICATION_TYPE = "QApplication";
static const QString NULL_TYPE = "Null";

class TasFixturePluginInterface  
{
public:
     virtual ~TasFixturePluginInterface() {}

     /*!

	Method for calling actions inside plugin.
	
     */
     virtual bool execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut) = 0;

};

 Q_DECLARE_INTERFACE(TasFixturePluginInterface,
                     "com.nokia.testability.TasFixturePluginInterface/1.0")

#endif 
