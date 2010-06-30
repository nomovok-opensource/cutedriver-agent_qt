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
 

#ifndef TASSERVERCOMMAND_H
#define TASSERVERCOMMAND_H

#include <tascommand.h>
#include "tasclientmanager.h"

class TasServerCommand : public TasServiceCommand
{
public:
    virtual ~TasServerCommand(){};

	/*!
	  From ServiceInterface
	*/
	virtual bool executeService(TasCommandModel& model, TasResponse& response) = 0;
	virtual QString serviceName()const = 0;

protected:
	TasCommand* getCommandParameters(TasCommandModel& commandModel, const QString& commandName);
};

#endif
