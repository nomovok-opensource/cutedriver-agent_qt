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
 

#ifndef TESTABILITY_H
#define TESTABILITY_H

#include <QObject>
#include <QtGui>
#include <QWidget>
#include "testabilityinterface.h"

class TestabilityPlugin : public QObject, public TestabilityInterface
{
    Q_OBJECT      
    Q_PLUGIN_METADATA(IID "com.nokia.testability.Testability" FILE "testability.json")
    Q_INTERFACES(TestabilityInterface)

public:
    TestabilityPlugin(QObject *parent = 0);    

    void Initialize();
private:
#ifdef Q_OS_SYMBIAN
	bool loadPlugin();
#endif
    static QString getApplicationName();
    static QString parseExecutable(QString completePath);


};

#endif
