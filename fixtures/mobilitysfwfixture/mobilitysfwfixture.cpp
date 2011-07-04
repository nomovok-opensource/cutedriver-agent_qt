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


#include <QApplication>
#include <QtPlugin>
#include <QDebug>
#include <QHash>

#include <taslogger.h>
#include <testabilityutils.h>
#include <tasqtdatamodel.h>

#include "mobilitysfwfixture.h"

#ifdef Q_OS_SYMBIAN
#include <qservicemanager.h>
#else
#include <QServiceManager>
#endif

QTM_USE_NAMESPACE


        Q_EXPORT_PLUGIN2(mobilitysfwfixture, MobilitySfwFixture)

        /*!
        \class MobilitySfwFixture
        \brief Collects frames per second data from widgets.
        
        Filters paint events to the given target and calculates fps.
        For graphicsitems the view's viewport is used.
        */

        /*!
        Constructor
        */
        MobilitySfwFixture::MobilitySfwFixture(QObject* parent)
            :QObject(parent)
{}

/*!
  Destructor
*/
MobilitySfwFixture::~MobilitySfwFixture()
{}

/*!
  Implementation for traverse so always true.
*/
bool MobilitySfwFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> /*parameters*/, QString & stdOut)
{
    Q_UNUSED(objectInstance);

    TasLogger::logger()->debug("> MobilitySfwFixture::execute:" + actionName);

    if (actionName == "list_services") {
        listServices(stdOut);
    }

    return true;
}

void MobilitySfwFixture::listServices(QString& stdOut)
{
    TasLogger::logger()->debug("> MobilitySfwFixture::listServices ");

    QServiceManager serviceManager;
    QStringList services = serviceManager.findServices();

    QStringListIterator it(services);

    while(it.hasNext()){
        stdOut.append(it.next());
        if(it.hasNext())
            stdOut.append(";");
    }

    //    TasLogger::logger()->debug("> MobilitySfwFixture::readFile \n" + stdOut);
}
