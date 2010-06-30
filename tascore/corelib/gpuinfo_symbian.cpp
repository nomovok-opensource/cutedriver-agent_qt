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
 

#include "gpuinfo_symbian.h"
#include "taslogger.h"

#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )

GpuMemDetailsHandler::GpuMemDetailsHandler()
{
    TasLogger::logger()->debug("GpuMemDetailsHandler::GpuMemDetailsHandler");
	prof_data = 0;
    iVendor = 0;
    iVersion = 0;
    iClientAPIS = 0;
    iExtensions = 0;
	numberofinfos = 0;
	totalmem = 0;
	usedmem = 0;
	usedmax = 0;
    usedmin = KMaxTInt;
    InitializeEGL();
}

GpuMemDetailsHandler::~GpuMemDetailsHandler()
{
    delete iVendor;
    delete iVersion;
    delete iClientAPIS;
    delete iExtensions;
    if (prof_data)
        {
        free(prof_data);
        }
    eglTerminate(iEGLDisplay);
}

GpuMemDetails GpuMemDetailsHandler::gpuData()
{
    GetProfileData();
    GpuMemDetails details;
    details.totalMem = totalmem;
    details.usedMem = usedmem;
    details.freeMem = totalmem - usedmem;
    details.isValid = true;

    details.processPrivateMem = 0;
    details.processSharedMem = 0;

    TInt64 processId = RProcess().Id();

    TBool foundAny( EFalse );
    TInt i( 0 );
    while( i < numberofinfos && !foundAny ){
        if ( processarray[i].id == processId ){
            details.processPrivateMem = processarray[i].privatemem;
            details.processSharedMem = processarray[i].sharedmem; 
            foundAny = ETrue;
        }
        i++;
    }
    return details;
}

HBufC16* GpuMemDetailsHandler::EGLGetString(EGLint aName)
    {
    const char* str = eglQueryString(iEGLDisplay, aName);
    if (!str)
        return NULL; 
    int len = strlen(str);
    char* copy = (char*)malloc(len*2);
    strncpy(copy,str,len);
    HBufC16* widestr = NULL;
    TPtr8 strptr((unsigned char*)copy, len, len*2);
    TPtr16 wideptr = strptr.Expand();
    widestr = wideptr.Alloc();
    free(copy);
    return widestr;
    }

void GpuMemDetailsHandler::InitializeEGL()
{
    iEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(iEGLDisplay, &iEGLMajor, &iEGLMinor);
    
    iVendor = EGLGetString(EGL_VENDOR);
    iVersion = EGLGetString(EGL_VERSION);
    iClientAPIS = EGLGetString(EGL_CLIENT_APIS);
    iExtensions = EGLGetString(EGL_EXTENSIONS);
    
    TLex lex(*iExtensions);
    while (!lex.Eos())
        {
        if (!lex.NextToken().CompareF(_L("EGL_NOK_resource_profiling")))
            {
            iProfileExtensionFound = ETrue;
            }
        }
    eglQueryProfilingData = (NOK_resource_profiling)eglGetProcAddress("eglQueryProfilingDataNOK");
    if (eglQueryProfilingData)
        {
        iProfileExtensionFound = ETrue;
        }
}


void GpuMemDetailsHandler::GetProfileData()
{
    numberofinfos = 0;
    
    if (!iProfileExtensionFound)
        {
            return;
        }
    eglQueryProfilingData(iEGLDisplay, 
                          EGL_PROF_QUERY_GLOBAL_BIT_NOK | 
                          EGL_PROF_QUERY_MEMORY_USAGE_BIT_NOK,
                          NULL,
                          0,
                          (EGLint*)&data_count);

    /* Allocate room for the profiling data */
    if (prof_data){
        free(prof_data);
    }

    /*(EGLint*)*/prof_data = (EGLint*)malloc(data_count * sizeof(EGLint));
    memset(prof_data,0,data_count * sizeof(EGLint));

    /* Retrieve the profiling data */
    eglQueryProfilingData(iEGLDisplay, 
                          EGL_PROF_QUERY_GLOBAL_BIT_NOK | 
                          EGL_PROF_QUERY_MEMORY_USAGE_BIT_NOK,
                          prof_data,
                          data_count,
                          (EGLint*)&data_count);

    
    EGLint i = 0;
    while (prof_data && i < data_count) {
        switch (prof_data[i++])
            {
            case EGL_PROF_TOTAL_MEMORY_NOK:
                {
                    totalmem = prof_data[i++];
                    break;
                }
            case EGL_PROF_USED_MEMORY_NOK:
                {
                    usedmem = prof_data[i++];
                    if (usedmem > usedmax){
                        usedmax = usedmem; 
                    }
                    if (usedmem < usedmin){
                        usedmin = usedmem;
                    }
                    
                    break;
                }
            case EGL_PROF_PROCESS_ID_NOK:
                {
                    numberofinfos++;
                    processarray[numberofinfos-1].id = ((EGLNativeThreadIdTypeNOK)(prof_data[i])) + (((EGLNativeThreadIdTypeNOK)(prof_data[i + 1]))<<32);
                    //processarray[numberofinfos].id = 0;
                    i+=2;
                    break;
                }
            case EGL_PROF_PROCESS_USED_PRIVATE_MEMORY_NOK:
                {
                    processarray[numberofinfos-1].privatemem = prof_data[i++];
                    break;
                }
            case EGL_PROF_PROCESS_USED_SHARED_MEMORY_NOK:
                {
                    processarray[numberofinfos-1].sharedmem = prof_data[i++];
                    break;
                }
            default:
                i++;
            }
    }
}

#endif
