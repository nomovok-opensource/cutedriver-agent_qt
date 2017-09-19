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


#include <QtGui>
#include <QLibrary>
#include <QDebug>

#include <sys/types.h>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#include <QCoreApplication>

#include "testability.h"


// Copied directly from tascoreutils. Do not want to link the lib
// to the plugin.
QString TestabilityPlugin::parseExecutable(QString completePath)
{
    QString name = "unk";

    completePath = completePath.replace("\\","/");
    QStringList list = completePath.split("/");
    name = list.last();
    name = name.remove(".exe", Qt::CaseInsensitive);

    return name;
}


QString TestabilityPlugin::getApplicationName()
{

#if defined(Q_OS_UNIX)
    QString name = "unk";
    unsigned bytes = -1;
    char buf[50];
    char buf_name[300];
    snprintf(buf, 50, "/proc/%u/cmdline", (unsigned)getpid());
    FILE* pf = fopen(buf, "r");

    if (pf) {
        bytes = fread(buf_name, 1, 300, pf);

        if (bytes) {
            name = buf_name;
        }
        fclose(pf);
    }
#else
    QString name = QCoreApplication::applicationFilePath();
#endif
    return name;
}


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
TestabilityPlugin::TestabilityPlugin(QObject* parent)
    :QObject(parent)
{
}



/*!

  Intitialize the plugin for use. Takes the tasService component into use.
  Starts the service and registers to the TasServer.

*/
void TestabilityPlugin::Initialize()
{
    // Ignore the launcher (but allow retrying)
    if (getApplicationName() == "applauncherd.bin" || getApplicationName() == "applifed.x" ||
        getApplicationName() == "applifed") {
        return;
    }
    QLibrary testLib("qttestability");
    if(testLib.load()){
        qDebug("TestabilityPlugin::Initialize() lib loaded");
        typedef void (*TasInitialize)(void);
        TasInitialize initFunction = (TasInitialize)testLib.resolve("qt_testability_init");
        if(initFunction){
            //qDebug("TestabilityPlugin::Initialize() call init");
            initFunction();
            //qDebug("TestabilityPlugin::Initialize() init called");
        }
        else{
            qWarning() << "TestabilityPlugin::Initialize() could not load testability: " << testLib.errorString();
        }
    }
    else{
        qWarning() << "TestabilityPlugin::Initialize() could not load testability library: " << testLib.errorString();
    }
}
