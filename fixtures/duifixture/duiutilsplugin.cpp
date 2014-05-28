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
 

#include <QtPlugin>
#include <QDebug>
#include <QHash>
#include <DuiWidget>
#include <DuiButton>
#include <tasqtdatamodel.h>
#include <taslogger.h>

#include "duiutilsplugin.h"
#include "duideviceprofile.h"

#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QMetaProperty>

Q_EXPORT_PLUGIN2(duiutilsplugin, DuiUtilsPlugin)

/*!
  \class DuiUtilsPlugin
  \brief DuiUtilsPlugin contains fixtures defined for Dui
        
*/

/*!
  Constructor
*/
    DuiUtilsPlugin::DuiUtilsPlugin(QObject* parent)
        :QObject(parent)
{}

/*!
  Destructor
*/
DuiUtilsPlugin::~DuiUtilsPlugin()
{}

/*!
  Implementation for traverse so always true.
*/
bool DuiUtilsPlugin::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    Q_UNUSED(parameters);
    TasLogger::logger()->debug("DuiUtilsPlugin::execute ");
    // place your own code below
    bool result = true;

    TasDataModel* model = new TasDataModel();
    TasLogger::logger()->debug("DuiUtilsPlugin::execute tasDataModel");

    TasObjectContainer& container = model->addNewObjectContainer(1, "1", "DuiUtils");
    TasLogger::logger()->debug("DuiUtilsPlugin::execute container");

    // set the stdOut if you wish to pass information back to Testability Driver
    if(actionName == "DeviceProfile"){
       TasLogger::logger()->debug("DuiUtilsPlugin::execute DeviceProfile");
       TasObject& profile = container.addNewObject(1, "DuiUtils", "DuiDeviceProfile");
       //profile.addAttribute("name", DuiDeviceProfile::instance()->name().toLatin1().data());
       profile.addAttribute("resolution_width",  QString::number(DuiDeviceProfile::instance()->resolution().width()));
       profile.addAttribute("resolution_height",  QString::number(DuiDeviceProfile::instance()->resolution().height()));
       //profile.addAttribute("screenrect_x",  QString::number(DuiDeviceProfile::instance()->screenRectF().x()));
       //profile.addAttribute("screenrect_y",  QString::number(DuiDeviceProfile::instance()->screenRectF().y()));
       //profile.addAttribute("screenrect_width",  QString::number(DuiDeviceProfile::instance()->screenRectF().width()));
       //profile.addAttribute("screenrect_height",  QString::number(DuiDeviceProfile::instance()->screenRectF().height()));
       //profile.addAttribute("width",  QString::number(DuiDeviceProfile::instance()->width()));
       //profile.addAttribute("height",  QString::number(DuiDeviceProfile::instance()->height()));
       profile.addAttribute("pixelsperinch_width",  QString::number(DuiDeviceProfile::instance()->pixelsPerInch().width()));
       profile.addAttribute("pixelsperinch_height",  QString::number(DuiDeviceProfile::instance()->pixelsPerInch().height()));
       //profile.addAttribute("orientation",  QString::number(DuiDeviceProfile::instance()->orientation()));
       //profile.addAttribute("name", DuiDeviceProfile::instance()->name().toLatin1().data());

       model->serializeModel(stdOut);
       delete model;

       result =  true;
    } else if (actionName == "Properties") {
        if (objectInstance != NULL && objectInstance != 0) {
            DuiWidget *w = objectInstance2DuiWidget(objectInstance);
            if ( w != NULL && w != 0) {
                const QMetaObject *mo = w->metaObject();

                // FIXME: not sure about the following paremeters of the folloing call:
                TasObject& profile = container.addNewObject(1, "DuiUtils", mo->className() );

                for (int x = mo->propertyOffset(); x< mo->propertyCount() ; x++ ) {
                    QMetaProperty mp = mo->property(x);
                    profile.addAttribute( mp.name(), mp.typeName());
                }
                model->serializeModel(stdOut);
                delete model;
                result =  true;
            }
        } else {
            stdOut = "Object Instance is not valid";
            result =  false;
        }
    } else{
        stdOut = "Unknown action";
        result =  false;
    }
    return result;
}

DuiWidget *DuiUtilsPlugin::objectInstance2DuiWidget(void *objectInstance) {
    QGraphicsItem *qgi = qgraphicsitem_cast<QGraphicsItem*>( (QGraphicsItem*) objectInstance );
    if (qgi != NULL && qgi != 0) {
        if (qgi->isWidget()) {
            QGraphicsWidget *qgw = qobject_cast<QGraphicsWidget*>( (QGraphicsWidget*) qgi);
            if (qgw != NULL && qgw != 0) {
                DuiWidget *dw = qobject_cast<DuiWidget*>(qgw);
                if (dw != NULL && dw != 0) {
                    return dw;
                }
            }
        }
    }
    return 0;
}
