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
 


#ifndef TASEXTENSIONINTERFACE_H
#define TASEXTENSIONINTERFACE_H

#include <QByteArray>
#include <QString>

#include <tasqtcommandmodel.h>

/*!
    \class TasExtensionInterface
    \brief TasExtensionInterface provides additional services that can be provided for the qttasserver to perform.
    
	TasExtensionInterface implementations will be loaded by the qttasserver when ui state is requested.
	Implementing classes can add platform specific details such as the ui state. 

*/
class TasExtensionInterface
{
 public:
     virtual ~TasExtensionInterface(){}
	 /*!
	   Provide application ui details not provided by the qt traversers. If the QByteArray returned is null it will not be appended
	   to the response message. Empty implementations should return a null QByteArray.
	  */
	 virtual QByteArray traverseApplication(const QString& pid, const QString& name, const QString& applicationUid) = 0;

	 /*!
	   Perform the commands described ib the command model. The requester socket can be used to respond to the caller
	   of the command. Returns false if the command is not handled. When handling a command return true and set the 
	   response message to the response QByteArray. Note that only on plugin is allowed to handle a command and respond.
	   So a true returns value will have an affect that no other plugin get's after it.
	  */
	 virtual bool performCommand(TasCommandModel& commandModel, QByteArray& response) = 0;
};

Q_DECLARE_INTERFACE(TasExtensionInterface,
					"com.nokia.testability.TasExtensionInterface/1.0")

#endif 
 
