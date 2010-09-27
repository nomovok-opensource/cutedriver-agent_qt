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
 


#ifndef TASSERVICEMANAGER_H
#define TASSERVICEMANAGER_H

#include <QObject>
#include <QList>

#include "tasqtcommandmodel.h"
#include "tassocket.h"
#include "tascommand.h"
#include "tasmessages.h"

class TasServiceManager : public RequestHandler
{

public:
    TasServiceManager();
    ~TasServiceManager();

	void registerCommand(TasServiceCommand* command);
	void serviceRequest(TasMessage& request, TasSocket* requester);

protected:
	virtual void handleServiceRequest(TasCommandModel& commandModel, TasSocket* requester, qint32 responseId);
	void performService(TasCommandModel& commandModel, TasResponse& response);	
	TasCommandModel* parseMessageString(const QString& messageBody, QString& errorMessage);
	virtual QString serviceErrorMessage(){return "QtTestabilityPlugin does not support the given service: ";}

protected:
	QList<TasServiceCommand*> mCommands;
};

#endif
