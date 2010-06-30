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
 


#define USE_TAS_OS_SPEC

#ifndef PLATFORMSERVICE_H
#define PLATFORMSERVICE_H

#include "tasservicebase.h"
#include "taslogger.h"

class PlatformService : public TasServiceBase 
{
public:
    PlatformService();
	~PlatformService();

	/*!
	  From ServiceInterface
	*/
	bool executeService(TasCommandModel& model, TasResponse& response);
	QString serviceName()const { return PLATFORM; }

private:	
	void doPlatformSpecificCommands(TasCommandModel& commandModel);
#ifdef USE_TAS_OS_SPEC
	void sendFileOpenMessage(const QString& dialogName, const QString& filePath, const QString& buttonName);
#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))
	wchar_t* converToWChar(const QString& aString);
        void pressEnter();
#endif
#endif
};

#endif
