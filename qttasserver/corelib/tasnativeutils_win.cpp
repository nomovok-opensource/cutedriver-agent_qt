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

int TasNativeUtils::bringAppToForeground(quint64 pid)
{
    Q_UNUSED(pid);
    return -1;
}

void TasNativeUtils::changeOrientation(QString)
{}

bool TasNativeUtils::killProcess(quint64 pid)
{
    HANDLE hProcess;
    hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, pid );
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
    hProcess = OpenProcess( READ_CONTROL, FALSE, pid );
    if( hProcess  ){
        CloseHandle(hProcess);
        running = true;
    }
    return running;
}

bool TasNativeUtils::processExitStatus(quint64 pid, int &status)
{
    bool stopped = true;
    HANDLE hProcess;
    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, pid );
    if( hProcess  ){
        DWORD dwExitCode = 0;
        if(GetExitCodeProcess(hProcess, &dwExitCode)){
            if(dwExitCode == STILL_ACTIVE){
                stopped =  false;            
            }
            else{
                status = dwExitCode;
            }
        }
        else{
            
            TasLogger::logger()->debug("TasNativeUtils::processExitStatus could not get status");
            //maybe no process since could not open
            status = 0;
        }
        CloseHandle(hProcess);
    }
    return stopped;
}

void TasNativeUtils::runningProcesses(TasObject& applist)

{
     // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        //return empty if fails
        return ;
    }
    cProcesses = cbNeeded / sizeof(DWORD);

    //get the details
    for ( i = 0; i < cProcesses; i++ ){
        if( aProcesses[i] != 0 ){
            DWORD processID = aProcesses[i];
            HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, processID );
            if (NULL != hProcess ){
                HMODULE hMod;
                DWORD cbNeeded;                
                TCHAR szProcessName[MAX_PATH] = TEXT("unknown");
                if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) ){
                    GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );
                }
                QString fullName;
#ifdef UNICODE
                fullName = QString::fromUtf16((ushort*)szProcessName);
#else
                fullName = QString::fromLocal8Bit(szProcessName);
#endif                

                QString processName = fullName.split(".exe").first();
                TasObject& processDetails = applist.addNewObject(QString::number(processID), processName, "process");
                //add mem
                PROCESS_MEMORY_COUNTERS pmc;
                if(GetProcessMemoryInfo(hProcess,&pmc, sizeof(pmc))){
                    processDetails.addAttribute("memUsage", (int)pmc.WorkingSetSize);
                    processDetails.addAttribute("fullName", fullName);
                }            
            }
            CloseHandle( hProcess );
        }
    }
}
