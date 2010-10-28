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


#ifndef UTILFIXTUREPLUGIN_H
#define UTILFIXTUREPLUGIN_H

#include <QObject>
#include <QHash>
#include <QSignalSpy>
#include <tasqtfixtureplugininterface.h>

#ifdef QTWRT_USE_USIF
#include <e32std.h>
#include <securewidgetregistryclient.h>
#include <securewidgetregistryconstants.h>
#include "../../../cwrt/wrt/widgetmanager/src/platform/symbian/SCRConstants.h"
#else
#include <WidgetRegistryClient.h>
#endif




class UtilFixturePlugin : public QObject, public TasFixturePluginInterface
{
  Q_OBJECT
  Q_INTERFACES(TasFixturePluginInterface)
 
public:
     UtilFixturePlugin(QObject* parent=0);
     ~UtilFixturePlugin();
     bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);
private:
     bool isWidgetRunning(QString appId);
     TBool isWidgetRunningL(QString& uniqueID, TUid& uiUid);
     QString getBundleIds();
     QString getInstalledWidgetAttributes();
     QString getBundleIDfromWidgetID(QString appId);
     QString getBundleNamefromWidgetID(QString appId);
     QString getUidFromBundleID(QString myBundleID);
     QString getWidgetNameFromWidgetUniqueID(QString appId);
     bool widgetInstall(const QString &path, bool silent, bool update, QObject *parent, const QString &rootDir);
     bool widgetUninstall(QString &uid, bool silent);
     bool WaitForSignal(QSignalSpy & spy_signal , int expected);
     
 };

#endif
 
