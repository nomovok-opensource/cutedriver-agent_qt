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
 


#include "tasnativeutils.h"
#include <taslogger.h>
#include <w32std.h>
#include <apgtask.h>
#include <QTimer>
#include <memspydriverclient.h>
#include <e32property.h>

/*!
  \class TasNativeUtils
  \brief Symbian platform specific implementation of the interface.
*/

int TasNativeUtils::pidOfActiveWindow(const QHash<QString, TasClient*> clients)
{
    TasLogger::logger()->debug("-> TasNativeUtils::pidOfActiveWindow");

    //do not bother searching if no apps reqistered
    if(clients.isEmpty()){
        return TAS_ERROR_NOT_FOUND;
    }
    
    //check for dialogs
    const TUid PropertyCategoryUid = {0x20022FC5};
    const TUint StatusKey = 'Stat';
    TInt shown = 0;
    RProperty::Get(PropertyCategoryUid, StatusKey, shown);
    if(shown == 1){
        foreach (TasClient* app, clients){
            if( app->applicationUid() == QString::number(PropertyCategoryUid.iUid)){
                return app->processId().toInt();
            }
        }
    }

    const QList<QString>& pids = clients.keys();
    int pid = TAS_ERROR_NOT_FOUND;
    RWsSession wsSession;
    int error = wsSession.Connect();
    if (KErrNone == error) {
        TApaTaskList applicationList(wsSession);
        
        int i = 0;
        int max = wsSession.NumWindowGroups();
        bool appExists = false;
        do {
            TApaTask foregroundApp = applicationList.FindByPos(i);
            appExists = foregroundApp.Exists();
            if (appExists) {
                RThread foregroundAppThread;
                if (KErrNone == foregroundAppThread.Open(foregroundApp.ThreadId().Id())) {
                    RProcess foregroundAppProcess;
                    foregroundAppThread.Process(foregroundAppProcess);
                    pid = foregroundAppProcess.Id().Id();
                    if (!pids.contains(QString::number(pid))) {
                        pid = TAS_ERROR_NOT_FOUND;
                    }
                    else {
                    }
                    foregroundAppThread.Close();
                } 
                else {
                    TasLogger::logger()->error(
                            "   Could not open topmost app thread, ID: " + 
                            QString::number(foregroundApp.ThreadId().Id()));
                }
            } 
            else {
                TasLogger::logger()->error("   No pos " + QString::number(i) + " app found!");
            }
            i++;
            //break the loop if max windowgroup reached
            //mysterious amount of tasks sometimes occurs causing infinite loops
            if( i > max){
                appExists = false;
            }
        } while (appExists && TAS_ERROR_NOT_FOUND == pid);

        wsSession.Close();    
    }
    else {
        TasLogger::logger()->error("   Couldn't connect to WS: " + QString::number(error));
    }
    TasLogger::logger()->debug("<- TasNativeUtils::pidOfActiveWindow");
    return pid;
}

int TasNativeUtils::bringAppToForeground(TasClient& client)
{
    TasLogger::logger()->debug("-> TasNativeUtils::bringAppToForeground");
    int error = TAS_ERROR_NONE;
    
    RWsSession wsSession;
    error = wsSession.Connect();
    if (KErrNone == error) {
        TasLogger::logger()->debug("   Connected to WS");
        TApaTaskList applicationList(wsSession);
    
        // Find app by UID
        bool ok = false;
        QString applicationUid = client.applicationUid();
        TUid appUid = TUid::Uid(applicationUid.toInt(&ok, 10));
        TApaTask app = applicationList.FindApp(appUid);
        if (app.Exists()) {
            app.BringToForeground();
        } 
        else {
            TasLogger::logger()->error("Application found but it does not exist!");
            error = TAS_ERROR_NOT_FOUND;
        }
        wsSession.Close();    
    }
    else {
        TasLogger::logger()->error("   Couldn't connect to WS: " + QString::number(error));
    }
    TasLogger::logger()->debug("<- TasNativeUtils::bringAppToForeground");
    return error;
}

