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



#ifndef TESTABILITYUSETTINGS_H
#define TESTABILITYUSETTINGS_H

#include <QObject>
#include <QSettings>
#include "tasconstants.h"

class TestabilitySettings
{
private:
    TestabilitySettings();
    ~TestabilitySettings();

public:
    static TestabilitySettings* settings();
    static void deleteInstance();

    QVariant getValue(const QString& key);
    bool setValue(const QString& key, const QVariant& value);
    static QString getBasePath(const QString &append=QString());

private:
    static TestabilitySettings *mInstance;
    QSettings* mSettings;

};

#endif
