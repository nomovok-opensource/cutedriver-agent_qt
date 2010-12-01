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

#include <QApplication>
#include <QtPlugin>
#include <QDebug>
#include <QHash>

#include <taslogger.h>
#include <testabilityutils.h>
#include <tasqtdatamodel.h>

#include "MobilitySysInfoFixture.h"


static const QString ACTION_GET_IMEI = "imei";
static const QString ACTION_GET_IMSI = "imsi";

Q_EXPORT_PLUGIN2(mobilitysysinfofixture, MobilitySysInfoFixture)

/*!
\class MobilitySysInfoFixture
\brief Manages contact on device

*/

/*!
Constructor
*/
MobilitySysInfoFixture::MobilitySysInfoFixture(QObject* parent)
    :QObject(parent)
{}

/*!
  Destructor
*/
MobilitySysInfoFixture::~MobilitySysInfoFixture()
{}

/*!
  Implementation for traverse so always true.
*/
bool MobilitySysInfoFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    Q_UNUSED(objectInstance);

    TasLogger::logger()->debug("> MobilitySysInfoFixture::execute:" + actionName);

    QSystemDeviceInfo * di = new QSystemDeviceInfo(this);

    bool retVal = false;

    if (actionName == ACTION_GET_IMEI) {
        stdOut.append(di->imei());
        retVal = true;
    } else if (actionName == ACTION_GET_IMSI) {
        stdOut.append(di->imsi());
        retVal = true;
    } else {
        stdOut.append("Invalid mobility sysinfo command: "+ actionName);
        return false;
    }

    return retVal;
}

