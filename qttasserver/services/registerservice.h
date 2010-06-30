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
 

#ifndef REGISTERSERVICE_H
#define REGISTERSERVICE_H

#include <QObject>
#include <QQueue>

#include <taspluginloader.h>
#include <tasconstants.h>

#include "tasservercommand.h"


struct ClientDetails
{
  QString processName;
  QString processId;
#ifdef Q_OS_SYMBIAN
  QString applicationUid;
#endif
  QString pluginType;
  TasSocket* socket;
};

class RegisterService : public QObject, public TasServerCommand
{
    Q_OBJECT
public:
    RegisterService();
    ~RegisterService();

	/*!
	  From ServiceInterface
	*/
	bool executeService(TasCommandModel& model, TasResponse& response);
	QString serviceName() const { return REGISTER; }

private slots:
	void registerQueuedClients();

private:
	void registerPlugin(TasCommand& command, TasResponse& response);
	void unRegisterPlugin(TasCommand& command);

private:
    QQueue<ClientDetails> mClientQueue;
};

#endif
