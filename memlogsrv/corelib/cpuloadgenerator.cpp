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

CpuLoadGenerator::CpuLoadGenerator()
{
    mRunning = false;
}

CpuLoadGenerator::~CpuLoadGenerator()
{}

int CpuLoadGenerator::start(int loadInPercentage)
{
    Q_UNUSED(loadInPercentage);
    return TAS_ERROR_NOT_IMPLEMENTED;
}

int CpuLoadGenerator::stop()
{
    return TAS_ERROR_NOT_IMPLEMENTED;
}
