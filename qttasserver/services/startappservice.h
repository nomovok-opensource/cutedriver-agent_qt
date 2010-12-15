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
 


#ifndef STARTAPPSERVICE_H
#define STARTAPPSERVICE_H

#include <QTimer>

#include <tasconstants.h>

#include "tasservercommand.h"

class StartAppService : public TasServerCommand
{
public:
    StartAppService();
    ~StartAppService();

	/*!
	  From ServiceInterface
	*/
	bool executeService(TasCommandModel& model, TasResponse& response);
	QString serviceName() const { return START_APPLICATION; }

#ifdef Q_OS_SYMBIAN
	static void failedRegister();
	static void successfullRegister();
#endif

private:
	void startApplication(TasCommand& command, TasResponse& response);
    void launchAttached(const QString& applicationPath,const QStringList& arguments, TasResponse& response, QHash<QString, QString> environmentVariables);
	void launchDetached(const QString& applicationPath,const QStringList& arguments, TasResponse& response, bool noWait);
	void setRuntimeParams(TasCommand& command);

private:
    QHash<QString, QString> parseEnvironmentVariables(const QString& env);

#ifdef Q_OS_SYMBIAN
	static int mFailedRegisterCount;
#endif

};
  
class RegisterWaiter : public QObject
{
  Q_OBJECT
public:
  RegisterWaiter(TasSocket* requester, TasClient *target, qint32 messageId, bool noWait=false);
	
private slots:
    void selfRegister();
	void clientRegistered(const QString& processId);
    void timeout();
	void socketClosed();
	void crashed();

private:
	QTimer mWaiter;
	QString mProcessId;
	QString mProcessName;
	qint32 mMessageId;
	TasSocket* mSocket;
};

#endif
