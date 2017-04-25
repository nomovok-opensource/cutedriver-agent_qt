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



#include <QGraphicsWidget>
#include <QMouseEvent>
#include <QLocale>

#include "localefixture.h"
#include "testabilityutils.h"
#include "taslogger.h"

static const QString DAYNUMBER = "daynumber";
static const QString MONTHNUMBER = "monthnumber";
static const QString TIMESTRING = "timestring";
static const QString DATESTRING = "datestring";
static const QString CURRENCYAMOUNT = "currencyamount";
static const QString CURRENCYTYPE = "currencytype";
static const QString NUMBERVALUE = "numbervalue";



/*!
  \class LocaleFixturePlugin
  \brief give access to QLocale/MLocale values from scripts
*/

LocaleFixturePlugin::LocaleFixturePlugin(QObject* parent) :QObject(parent) {}
LocaleFixturePlugin::~LocaleFixturePlugin() {}

bool LocaleFixturePlugin::execute(
    void* objectInstance,
    QString actionName,
    QHash<QString, QString> parameters,
    QString& stdOut)
{
    TasLogger::logger()->debug("> LocaleFixturePlugin::execute translation");
    bool result = true;
    stdOut.append("LocaleFixture Not implemented");
    return false;
}
