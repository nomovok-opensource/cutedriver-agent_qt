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



#ifndef SETTINGFIXTUREPLUGIN_H
#define SETTINGFIXTUREPLUGIN_H

#include <QSettings>
#include <QObject>
#include <tasqtfixtureplugininterface.h>


class SettingFixture : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.nokia.testability.SettingFixture" FILE "settingfixture.json")
    Q_INTERFACES(TasFixturePluginInterface)

public:
     SettingFixture(QObject* parent=0);
     ~SettingFixture();
     bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);

private:
     QString editSettings(QSettings& settings, const QString& action, QHash<QString, QString> parameters);
     QSettings::Format getFormat(QHash<QString, QString>& parameters);
     QSettings::Scope getScope(QHash<QString, QString>& parameters);
};

#endif

