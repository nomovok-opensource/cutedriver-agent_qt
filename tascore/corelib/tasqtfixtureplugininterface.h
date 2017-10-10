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

const char* const OBJECT_TYPE = "__fixture_object_type";
const char* const WIDGET_TYPE = "QWidget";
const char* const QQUICKITEM_TYPE = "QQuickItem";
const char* const QQUICKVIEW_TYPE = "QQuickView";
const char* const QQUICKWINDOW_TYPE = "QQuickWindow";
const char* const GRAPHICS_ITEM_TYPE = "QGraphicsItem";
const char* const APPLICATION_TYPE = "QApplication";
const char* const NULL_TYPE = "Null";

const char* const CUCUMBER_STEP_DEFAULTACTION = "cucumberStep";

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
