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

#include <tasqtdatamodel.h>
#include <taslogger.h>
#include <tascoreutils.h>
#include <version.h>
#include <tasdeviceutils.h>

#include "tasclientmanager.h"
#include "uistateservice.h"

UiStateService::UiStateService()
{}

UiStateService::~UiStateService()
{}

bool UiStateService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() || model.service() == FIND_OBJECT_SERVICE){
        response.setData(serverUiState());
        response.setIsError(false);
        return true;
    }
    else{
        return false;
    }
}

/*!
  Send the server ui state.
*/
QByteArray UiStateService::serverUiState()
{
    TasLogger::logger()->debug("TasServer::serverUiState");
    TasDataModel* model = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = model->addNewObjectContainer(1, qtVersion, "qt");

    QString name = TasCoreUtils::getApplicationName();
    TasObject& application = container.addNewObject(QString::number(QCoreApplication::applicationPid()), name, "application");
    application.setEnv("qt");
    application.addAttribute("arguments", qApp->arguments().join(" ").toLatin1().data());
    application.addAttribute("exepath", qApp->applicationFilePath().toLatin1().data());
    application.addAttribute("FullName", qApp->applicationFilePath().toLatin1().data());
    application.addAttribute("dirpath", qApp->applicationDirPath().toLatin1().data());
    application.addAttribute("processId", QString::number(qApp->applicationPid()).toLatin1().data());
    application.addAttribute("version", qApp->applicationVersion().toLatin1().data());
    application.addAttribute("objectType", TYPE_APPLICATION_VIEW);

    int mem = TasDeviceUtils::currentProcessHeapSize();
    if(mem != -1){
        application.addAttribute("memUsage", mem);
    }

    QByteArray xml;
    model->serializeModel(xml);
    delete model;
    return xml;
}

