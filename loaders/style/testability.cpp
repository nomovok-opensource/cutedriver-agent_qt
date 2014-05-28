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
 

#include <QLibrary>
#include <QDebug>

#include "testability.h"



/*!
    \class TestabilityPlugin
    \brief TestabilityPlugin component is loaded into testable application usually in the qapplication component.     
    
    TestabilityPlugin component is loaded into testable application usually in the qapplication component.
    
    TestabilityPlugin component starts using the TestabilityService component to register it self to the TasServer
    running on the target (needs to be running to enable testing). After the register process the TasServer 
    sends command and ui state requests to the this application.    
     
*/

/*!
    Constucts the plugin and initializes all needed services. 
*/
TestabilityPlugin::TestabilityPlugin(QObject * parent )
    :QStylePlugin(parent)
{     
    QLibrary testLib("qttestability");
    if(testLib.load()){
        typedef void (*TasInitialize)(void);
        TasInitialize initFunction = (TasInitialize)testLib.resolve("qt_testability_init");
        #ifdef Q_OS_SYMBIAN
        //not found so use ordinal
        if(!initFunction){
            initFunction = (TasInitialize)testLib.resolve("1");
        }
        #endif
        if(initFunction){            
            initFunction();
        }
        else{
            qWarning() << "TestabilityPlugin::Initialize() could not load testability: " << testLib.errorString();
        }
    }
    else{
        qWarning() << "TestabilityPlugin::Initialize() could not load testability library: " << testLib.errorString();
    }
}

QStyle* TestabilityPlugin::create ( const QString & /*key*/ )
{
    return 0;
}

QStringList TestabilityPlugin::keys() const
{
    return QStringList() << "testability";
}

