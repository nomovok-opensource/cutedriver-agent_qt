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

#include "cpuloadgenerator.h"
#include <taslogger.h>
#include <QString>
#include <e32math.h>

_LIT(LoadGeneratorThreadName, "QtTasMemLogSrv_CpuLoadGenerator");

TInt LoadGeneratingThreadFunc(TAny* aLoadInPercentage)
{
    // The requested CPU load
    TInt* paramPtr = static_cast<TInt*>(aLoadInPercentage);
    TReal load = *paramPtr / 100;
    
    // Initial time
    TTime time1;
    time1.HomeTime();
    
    // Initial used CPU time
    TTimeIntervalMicroSeconds cpuTime1;
    RThread currentThread;
    TInt error = currentThread.GetCpuTime(cpuTime1);
    if (error) {
        currentThread.Panic(LoadGeneratorThreadName(), error);
    }
    
    TInt sleepTime = 100;
    while(ETrue) {
        // Calculate something relatively time consuming 
        TReal x = 4;
        TReal y = 0;
        for(TInt i = 0; i < 100000; i++) {
            Math::Sqrt(y, x);
            Math::Pow(x, y, 2);
        }
        
        // Current time
        TTime time2;
        time2.HomeTime();
        
        // Calculate time difference
        TTimeIntervalMicroSeconds timeDiff = time2.Int64() - time1.Int64();
        TReal64 timeDiffDouble = timeDiff.Int64();
        time1 = time2;
                
        // Current used CPU time
        TTimeIntervalMicroSeconds cpuTime2;
        error = currentThread.GetCpuTime(cpuTime2);
        if (error) {
            currentThread.Panic(LoadGeneratorThreadName(), error);
        }
        
        // Calculate CPU time difference
        TTimeIntervalMicroSeconds cpuDiff = cpuTime2.Int64() - cpuTime1.Int64();
        TReal64 cpuDiffDouble = cpuDiff.Int64();
        cpuTime1 = cpuTime2;
        
        // Calculate CPU usage
        TReal cpuUsage = cpuDiffDouble / timeDiffDouble;

        // Calculate time to sleep to achieve the requested load
        sleepTime = sleepTime * cpuUsage / load + 0.5;
        User::AfterHighRes(sleepTime * 1000);
    }
}

CpuLoadGenerator::CpuLoadGenerator()
{}

CpuLoadGenerator::~CpuLoadGenerator() 
{
    if (mLoadGenerationThread.ExitType() == EExitPending) {
        mLoadGenerationThread.Kill(KErrNone);        
    }
}

int CpuLoadGenerator::start(int loadInPercentage)
{
    mLoadInPercentage = loadInPercentage;
    int error = mLoadGenerationThread.Create(
                    LoadGeneratorThreadName(), 
                    LoadGeneratingThreadFunc, 
                    0x2000, 
                    NULL, 
                    &mLoadInPercentage);
    if (!error) {
        // SetPriority? Normal by default
        mLoadGenerationThread.Resume();
    }
    return error;
}

int CpuLoadGenerator::stop()
{
    if (mLoadGenerationThread.ExitType() == EExitPending) {
        mLoadGenerationThread.Kill(KErrNone);
    }
    return mLoadGenerationThread.ExitReason();
}
