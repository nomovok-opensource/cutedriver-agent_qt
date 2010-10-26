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

#ifndef CPULOADGENERATOR_H_
#define CPULOADGENERATOR_H_

#include <tasconstants.h>
#include <QObject>
#include <QTime>

class CpuLoadGenerator : public QObject
{
    Q_OBJECT
    
public:
    CpuLoadGenerator();
    ~CpuLoadGenerator();
    int start(int loadInPercentage);
    int stop();
    
private:

private: // Data
    int mLoadInPercentage;

private: // Platform specific data
#ifdef Q_OS_SYMBIAN
    RThread mLoadGeneratingThread;    
#endif
    
};

#endif /* CPULOADGENERATOR_H_ */
