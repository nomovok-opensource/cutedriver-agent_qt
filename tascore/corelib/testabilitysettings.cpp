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
 


#include <QDir>

#include "testabilitysettings.h"
#include "taslogger.h"

TestabilitySettings *TestabilitySettings::mInstance = 0;

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
static QString basePath = "/etc/qt_testability";
static QString fileName = "/etc/qt_testability/qt_testability.ini";
#elif defined(Q_OS_SYMBIAN)
//in symbian we need to check that drive based on installation (c, or z)
static QString basePath = "c:/qt_testability";
static QString fileName = "c:/qt_testability/qt_testability.ini";
static QString romFileName = "z:/qt_testability/qt_testability.ini";
#else
static QString basePath = "c:/qttas";
static QString fileName = "/qttas/conf/qt_testability.ini";
#endif

TestabilitySettings::TestabilitySettings()
{
#if defined(Q_OS_SYMBIAN)
    if(!QFile::exists(fileName) && QFile::exists(romFileName)){
        //copy from rom to c
        QDir dir("c:/");
        dir.mkpath("c:/qt_testability/");
        QFile::copy(romFileName, fileName);
        QFile::setPermissions(fileName, QFile::ReadOther | QFile::WriteOther | QFile::ReadOwner | QFile::WriteOwner);
#warning Does not copy cucumber step defintions yet!
    }
#endif
    mSettings = new QSettings(fileName, QSettings::IniFormat);
}

TestabilitySettings::~TestabilitySettings()
{
    delete mSettings;
}

TestabilitySettings* TestabilitySettings::settings()
{
    if(mInstance == 0){
        mInstance = new TestabilitySettings();
    }
    return mInstance;
}

void TestabilitySettings::deleteInstance()
{
    if(mInstance)
        delete mInstance;
}

QVariant TestabilitySettings::getValue(const QString& key)
{
    return mSettings->value(key);
}

bool TestabilitySettings::setValue(const QString& key, const QVariant& value)
{
    bool wasSet = false;
    if(mSettings->isWritable()){
        mSettings->setValue(key, value);
        mSettings->sync();
        wasSet =  true;
    }
    return wasSet;
}

QString TestabilitySettings::getBasePath(const QString &append)
{
    if (append.isEmpty()) return basePath;
    else return basePath + "/" + append;
}
