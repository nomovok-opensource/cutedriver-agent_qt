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



int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);    
    QCoreApplication::setOrganizationName("Nokia");
    QCoreApplication::setOrganizationDomain("nokia.com");
    QCoreApplication::setApplicationName("QtTasserver");


    // TEMP DEBUG
    TasLogger::logger()->setLogFile("qttasserver.log");
    TasLogger::logger()->setLevel(DEBUG);
    TasLogger::logger()->debug("Logger created");


    // Check Arguments for binding style
    QString hostBinding;
    if(app.arguments().count() > 1){
        hostBinding = app.arguments()[1];
    }


    // TODO Add settings verification
    QSettings settings("qttasserver.ini", QSettings::IniFormat);
    QString interfaceType = settings.value("interfaceType").toString();
    if (interfaceType.isEmpty()) {
        interfaceType = "local"; // Using Default, other is "any"
        settings.setValue("interfaceType", interfaceType);
    }

    TasLogger::logger()->debug(QString("MAIN::SETTINGS ") + interfaceType);

    
    TasServer* server = new TasServer(hostBinding);
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



