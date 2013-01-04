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


#ifndef LaunchFixturePLUGIN_H
#define LaunchFixturePLUGIN_H


#include <QObject>
#include <QHash>
#include <tasqtfixtureplugininterface.h>

class LaunchFixture : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT    
    Q_PLUGIN_METADATA(IID "com.nokia.testability.LaunchFixture" FILE "launchfixture.json")
    Q_INTERFACES(TasFixturePluginInterface)
public:
     LaunchFixture(QObject* parent=0);
     ~LaunchFixture();
     bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);

private:
 };

#endif //LaunchFixturePLUGIN_H
 
