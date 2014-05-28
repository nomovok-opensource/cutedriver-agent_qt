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
 


#ifndef TASSERVICEBASE_H
#define TASSERVICEBASE_H

#include <QObject>
#include <QtGui>
#include <QWidget>
#include <QAction>

#include "tascommand.h"

#include "testabilityutils.h"

class TasServiceBase : public TasServiceCommand, public TestabilityUtils
{
public:
    virtual ~TasServiceBase(){};

	/*!
	  From ServiceInterface
	*/
	virtual bool executeService(TasCommandModel& model, TasResponse& response) = 0;
	virtual QString serviceName()const = 0;
};

#endif
