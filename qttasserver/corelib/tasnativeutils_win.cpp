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
#include <windows.h>
#include <stdio.h>
#include <psapi.h>


int TasNativeUtils::pidOfActiveWindow(const QHash<QString, TasClient*> clients)
{
    Q_UNUSED(clients);
    return -1;
}

int TasNativeUtils::bringAppToForeground(TasClient& app)
{
    Q_UNUSED(app);
    return -1;
}

void TasNativeUtils::changeOrientation(QString)
{}

bool TasNativeUtils::killProcess(quint64 pid)
{
    HANDLE hProcess;
    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
    if( hProcess  ){
        TerminateProcess( hProcess, 0);
        CloseHandle(hProcess);
        return true;
    }
    return false;
}

bool TasNativeUtils::verifyProcess(quint64 pid)
{
    bool running = false;
    HANDLE hProcess;
    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
    if( hProcess  ){
        CloseHandle(hProcess);
        running = true;
    }
    return running;
}

bool TasNativeUtils::processExitStatus(quint64 pid, int &status)
{
    int code = 0;
    HANDLE hProcess;
    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
    if( hProcess  ){
        DWORD dwExitCode = 0;
        if(GetExitCodeProcess(hProcess, &dwExitCode)){
            TasLogger::logger()->debug("TasNativeUtils::processExitStatus " + QString::number(dwExitCode));
        }
        status = dwExitCode;
        CloseHandle(hProcess);
    }
    return true;
}
