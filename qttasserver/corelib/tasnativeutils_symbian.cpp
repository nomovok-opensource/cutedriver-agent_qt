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

//rotation include and defines
#include <cfclient.h>
_LIT( KContextSource, "Sensor" );
_LIT( KSensorSourceEventOrientation, "Event.Orientation" );
_LIT( KContextValueRightUp, "DisplayRightUp" );
_LIT( KContextValueLeftUp, "DisplayLeftUp" );
_LIT( KContextValueTopUp, "DisplayUp" );
_LIT( KContextValueDownUp, "DisplayDown" );


class NativeUtils_p
{
public:
    static TBool bringAppToForeground(TApaTask app);
    static TInt getUidForPid(const quint64& pid, TInt& uid);
};

/*!
  \class TasNativeUtils
  \brief Symbian platform specific implementation of the interface.
*/

int TasNativeUtils::pidOfActiveWindow(const QHash<quint64, TasClient*> clients)
{
    Q_UNUSED(clients);

    TasLogger::logger()->debug("-> TasNativeUtils::pidOfActiveWindow"); 
    int pid = TAS_ERROR_NOT_FOUND;
    RWsSession wsSession;
    int error = wsSession.Connect();
    if (KErrNone == error) {
        TApaTaskList applicationList(wsSession);        
        int i = 0;
        int max = wsSession.NumWindowGroups();
        TApaTask foregroundApp = applicationList.FindByPos(i);
        if ( foregroundApp.Exists() ) {
            RThread foregroundAppThread;
            if (KErrNone == foregroundAppThread.Open(foregroundApp.ThreadId().Id())) {
                RProcess foregroundAppProcess;
                if(foregroundAppThread.Process(foregroundAppProcess) == KErrNone){
                    pid = foregroundAppProcess.Id().Id();
                    foregroundAppProcess.Close();
                }
                foregroundAppThread.Close();
            } 
        } 
        wsSession.Close();    
    }
    else {
        TasLogger::logger()->error("   Couldn't connect to WS: " + QString::number(error));
    }
    TasLogger::logger()->debug("<- TasNativeUtils::pidOfActiveWindow");
    return pid;
}

int TasNativeUtils::bringAppToForeground(quint64 pid)
{
    TasLogger::logger()->debug("-> TasNativeUtils::bringAppToForeground");
    int error = TAS_ERROR_NONE;
    
    RWsSession wsSession;
    error = wsSession.Connect();
    if (KErrNone == error) {
        TasLogger::logger()->debug("   Connected to WS");
        TApaTaskList applicationList(wsSession);
        TInt uid;        
        if(NativeUtils_p::getUidForPid(pid, uid) == KErrNone){
            TasLogger::logger()->debug("App uid ok look for app");
            TUid appUid = TUid::Uid(uid);
            if(!NativeUtils_p::bringAppToForeground(applicationList.FindApp(appUid))){
                TasLogger::logger()->error("Application found but it does not exist!");
                error = TAS_ERROR_NOT_FOUND;
            }
        }
        else{
            TasLogger::logger()->error("Application could not be found for the pid");
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

void TasNativeUtils::changeOrientation(QString direction)
{
    //T R A P D starts
    TRAPD(err,
          MCFListener *cfListener= NULL;
          CCFClient* client = CCFClient::NewLC( *cfListener );
          CCFContextObject* co = CCFContextObject::NewLC();
          co->SetSourceL( KContextSource() );
          co->SetTypeL( KSensorSourceEventOrientation() );          
          if(direction == "rotate_right_up"){
              co->SetValueL( KContextValueRightUp() );
          }         
          else if(direction == "rotate_left_up"){
              co->SetValueL( KContextValueLeftUp() );
          }
          else if(direction == "rotate_down_up"){
              co->SetValueL( KContextValueDownUp() );
          }
          else{ // (direction == "rotate_top_up") as default
              co->SetValueL( KContextValueTopUp() );
          }
          TInt err = client->PublishContext( *co );
          CleanupStack::PopAndDestroy(2);
    );
    if( err != KErrNone ){
        TasLogger::logger()->error("<- TasNativeUtils::changeOrientation orientation changed failed, code: " + QString::number(err));
    }
    // T R A P D ends
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

TInt NativeUtils_p::getUidForPid(const quint64& pid, TInt& uid)
{
    TInt err = KErrNone;
    RProcess process;
    err = process.Open( TProcessId( pid ) );
    if( err == KErrNone){
        uid = process.Type().MostDerived().iUid;
        process.Close();            
    }            
    return err;
}


bool TasNativeUtils::killProcess(quint64 pid)
{
    bool killed = false;
    RProcess process;
    if( process.Open( TProcessId( pid ) ) == KErrNone ){
        if( process.ExitType() == EExitPending ){
            process.Kill(0);
            killed = true;
        }
        process.Close();
    }    
    return killed;
}
 

bool TasNativeUtils::verifyProcess(quint64 pid)
{
    bool running = false;
    RProcess process;
    if( process.Open( TProcessId( pid ) ) == KErrNone ){
        if( process.ExitType() == EExitPending ){
            running = true;
        }
        process.Close();
    }
    return running;
}

bool TasNativeUtils::processExitStatus(quint64 pid, int &status)
{
    bool stopped = true;
    status = 0;
    RProcess process;
    TInt code;
    if( process.Open( TProcessId( pid ) ) == KErrNone ){
        code = process.ExitType();
        if( code == EExitPending ){
            stopped = false;
        }
        else if( code == EExitPanic ){
            status = process.ExitReason();
        }
        else {
            status = 0;
        }
        process.Close();
    }
    return stopped;
}


void TasNativeUtils::runningProcesses(TasObject& applist)
{
    TFindProcess find;
  	TFullName res;
  	while(find.Next(res) == KErrNone) {
        RProcess process;
        if(process.Open(find) == KErrNone){
            if( process.ExitType() == EExitPending ){
                QString fullName = QString::fromUtf16(process.FileName().Ptr(), process.FileName().Length());
                QString processName = fullName.split("\\").last();
                processName = processName.split(".exe").first();
                TasObject& processDetails = applist.addNewObject(QString::number(process.Id().Id()), processName, "process");
                //try getting some kind of ram usage
                TProcessMemoryInfo memInfo;
                if(process.GetMemoryInfo(memInfo) == KErrNone){
                    TUint32 memSize = (memInfo.iCodeSize + memInfo.iConstDataSize + memInfo.iInitialisedDataSize + memInfo.iUninitialisedDataSize);
                    processDetails.addAttribute("memUsage", QString::number(memSize));
                    processDetails.addAttribute("fullName", fullName);
                }
            }
            process.Close();
        }
    }
}


