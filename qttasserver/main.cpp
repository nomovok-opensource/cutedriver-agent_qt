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


#include <QCoreApplication>
#include <QSettings>

#include "tasserver.h"

#include <taslogger.h>
#include <testabilitysettings.h>



int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setOrganizationDomain("nokia.com");
    QCoreApplication::setApplicationName("QtTasserver");

    qRegisterMetaType<quintptr>("quintptr");

    // Get arguments for binding style
    // No argument check done, only "any" or "localhost" will have an effect,
    // other settings will just be ignored and default host binding for current OS will be used
    QString argHostBinding = "";
    if(app.arguments().count() > 1){
        argHostBinding = app.arguments()[1];
    }

    // Update Persistent Settings if argument provided
    // Use persistent settings when no argument provided
    //QSettings settings("qttasserver.ini", QSettings::IniFormat);
    TestabilitySettings *settings = TestabilitySettings::settings();
    //QString settHostBinding = settings->value("hostBinding").toString();
    QString settHostBinding = settings->getValue("hostBinding").toString();
    if (!argHostBinding.isEmpty()){
        settHostBinding = argHostBinding;
        settings->setValue("hostBinding", settHostBinding);
    }

    TasServer* server = new TasServer(settHostBinding);
    if (server->startServer()){
        int code = app.exec();
        server->deleteLater();
        return code;
    }
    else{
        app.exit(0);
        return 0;
    }
}



