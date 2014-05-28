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
 

#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )

#ifndef GPUINFO_H
#define GPUINFO_H

// INCLUDES
#include <stdlib.h>
#include <string.h>

#include <e32std.h>
#include <e32cmn.h>
#include <e32des16.h>
#include <EGL/egl.h>

#include "tasdeviceutils.h"

// CONSTANTS
#define EGL_PROF_QUERY_GLOBAL_BIT_NOK           0x0001
#define EGL_PROF_QUERY_MEMORY_USAGE_BIT_NOK     0x0002

#define EGL_PROF_TOTAL_MEMORY_NOK               0x3070
#define EGL_PROF_USED_MEMORY_NOK                0x3071
#define EGL_PROF_PROCESS_ID_NOK                 0x3072
#define EGL_PROF_PROCESS_USED_PRIVATE_MEMORY_NOK 0x3073
#define EGL_PROF_PROCESS_USED_SHARED_MEMORY_NOK 0x3074

typedef EGLBoolean (*NOK_resource_profiling)(EGLDisplay, EGLint, EGLint*, EGLint, EGLint*);
typedef unsigned long long int EGLNativeThreadIdTypeNOK;

class GpuMemDetailsHandler : public GpuMemDetailsInterface
{
public:
    GpuMemDetailsHandler();
	~GpuMemDetailsHandler();

	GpuMemDetails gpuData();
	
private:	
	HBufC16* EGLGetString( EGLint aName );
	void InitializeEGL();
	void GetProfileData();

private:
	EGLDisplay iEGLDisplay;
	EGLint iEGLMajor;
	EGLint iEGLMinor;
	EGLint data_count;
	EGLint* prof_data;
	HBufC16* iVendor;
	HBufC16* iVersion;
	HBufC16* iClientAPIS;
	HBufC16* iExtensions;
	TBool iProfileExtensionFound;
	NOK_resource_profiling eglQueryProfilingData;
	TBool iProfilingStarted;
        
	typedef struct
	{
	  EGLNativeThreadIdTypeNOK id;
	  int sharedmem;
	  int privatemem;
	} processinfo_t;

	processinfo_t processarray[40];
	unsigned int numberofinfos;
	unsigned int totalmem;
	unsigned int usedmem;
	unsigned int usedmin;
	unsigned int usedmax;
};
#endif

#endif
