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

#ifndef CPULOADSERVICE_H_
#define CPULOADSERVICE_H_

#include <tascommand.h>
#include <tasconstants.h>

class CpuLoadGenerator;

class CpuLoadService : public QObject, public TasServiceCommand
{
    Q_OBJECT

public:
    CpuLoadService();
    ~CpuLoadService();
    bool executeService(TasCommandModel& model, TasResponse& response);
    QString serviceName() const { return CPU_LOAD_SERVICE; }
    
public slots:

private:
    int startLoad(TasCommand& command);
    int stopLoad(TasCommand& command, QString& responseData);
    bool commandIs(TasCommandModel& commandModel, const QString& commandName, TasCommand*& command);

private:
    CpuLoadGenerator* mLoadGenerator;
    
};

#endif /* CPULOADSERVICE_H_ */
