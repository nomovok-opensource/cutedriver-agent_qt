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
#include <cfclient.h>

class NativeUtils_p
{
public:
    static TBool bringAppToForeground(TApaTask app);
};

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
                    if(foregroundAppThread.Process(foregroundAppProcess) == KErrNone){
                        pid = foregroundAppProcess.Id().Id();
                        foregroundAppProcess.Close();
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
        TBool isOk = EFalse;
        // Find app by UID
        QString applicationUid = client.applicationUid();
        int uidi = applicationUid.toInt();
        //we must chect the uid since in some situation the uid seems 
        //to be too big for TInt32 which TUId.iUid is
        //name used if uid cannot be
        if(uidi != 0){
            TasLogger::logger()->debug("App uid ok look for app");
            TUid appUid = TUid::Uid(uidi);
            isOk = NativeUtils_p::bringAppToForeground(applicationList.FindApp(appUid));            
        }
        else{
            TasLogger::logger()->debug("App uid invalid look with name " + client.applicationName());
            TPtrC16 str(reinterpret_cast<const TUint16*>(client.applicationName().utf16()));            
            HBufC* buffer = str.Alloc(); 
            isOk = NativeUtils_p::bringAppToForeground(applicationList.FindApp(*buffer));
            delete buffer;            
        }        

        if(!isOk){
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

TBool NativeUtils_p::bringAppToForeground(TApaTask app)
{
    TBool value = EFalse;
    if (app.Exists()) {
        TasLogger::logger()->debug("App found bring to foreground");
        app.BringToForeground();
        value = ETrue;
    }
    return value;
}

void TasNativeUtils::changeOrientation()
{
    _LIT( KContextSource, "Sensor" );
    _LIT( KSensorSourceEventOrientation, "Event.Orientation" );
    _LIT( KContextValue, "DisplayRightUp?" );
    CCFContextObject* co = CCFContextObject::NewLC();
    co->SetSourceL( KContextSource );
    co->SetTypeL( KSensorSourceEventOrientation );
    co->SetValueL( KContextValue );
    CCFClient* client = CCFClient::NewLC( *this );
    TInt err = client->PublishContext( *co );
    if( err != KErrNone )
        {
        TasLogger::logger()->error("TasNativeUtils::changeOrientation failed: " +  QString::number(err));
        }
    CleanupStack::PopAndDestroy( client );
    CleanupStack::PopAndDestroy( co );
}
