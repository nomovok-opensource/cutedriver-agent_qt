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
 


#ifndef TASCLIENTMANAGER_H
#define TASCLIENTMANAGER_H

#include <QObject>
#include <QProcess>
#include <QHash>
#include <QList>
#include <QListIterator>
#include <QMutex>
#include <QTime>
#include <tassocket.h>
#include <tasdatashare.h>
#include <tasqtdatamodel.h>
#include <tasqtcommandmodel.h>

class TasClient;

class TasClientManager : public QObject
{
    Q_OBJECT

private:
  TasClientManager();
  ~TasClientManager();

public:
	
	enum ClientError {
		NoError,
		InsertError,
		QueryError,
		RemoveError
	};

	enum CloseStatus {
   	    Ok,
		Stalled,
		Crashed
	};

  static TasClientManager* instance();
  static void deleteInstance();

  TasClient* addClient(const QString& processId, const QString& processName=QString(), QProcess *process=0);  
  TasClient* addRegisteredClient(const QString& processId, const QString& processName, TasSocket* socket, const QString& type=TAS_PLUGIN);  

  TasClientManager::CloseStatus removeClient(const QString& processId, bool kill=false, int time=0);
  void removeAllClients();

  void applicationList(TasObject& parent);

  bool writeStartupData(const QString& identifier, const TasSharedData& data);
  bool detachFromStartupData(const QString& identifier);

  TasClient* findClient(TasCommandModel& request);
  TasClient* findByProcessId(const QString& processId, bool includeSocketLess=false);
#ifdef Q_OS_SYMBIAN
  TasClient* findByApplicationUid(const QString applicationUid);
#endif  
  TasClient* findCrashedApplicationById(const QString& processId);

  TasClientManager::ClientError moveClientToCrashedList(const QString& processId);
  void crashedApplicationList(TasObject& parent);
  void emptyCrashedApplicationList();
  TasClient* logMemClient();

signals:
  void allClientsRemoved();

private:

  TasClient* findByApplicationName(const QString& applicationName, bool includeSocketLess=false);
  TasClient* latestClient(bool includeSocketLess=false);

  void removeGhostClients();
  void checkJammedProcesses();
  TasClient* removeByProcessId(const QString& processId);  

private:
  QHash<QString, TasClient*> mClients;
  QHash<QString, QProcess*> mJammedProcesses;
  QHash<QString, TasClient*> mCrashedProcesses;
  static TasClientManager* mInstance;
  TasDataShare* mDataShare;
  QMutex mMutex;
  qint32 mMessageCounter;
};

class TasClient : public QObject
{
  Q_OBJECT
protected:
  TasClient(const QString& processId);
  ~TasClient();

public:
  const QString& processId() const;

  void setProcess(QProcess* process);
  QProcess* process();  
  void setSocket(TasSocket* socket);
  TasSocket* socket();
  void setApplicationName(const QString& applicationName);
  void killProcess();
  const QString& applicationName();
  int upTime();
  bool operator ==(const TasClient &client) const;
  void setRegistered();
  void closeConnection();
  QDateTime crashTime();
  void stopProcessMonitor();
#ifdef Q_OS_SYMBIAN
  void setApplicationUid(const QString& uid);
  QString applicationUid();
#endif
  void setPluginType(const QString& pluginType);
  QString pluginType();



signals:
  void registered(const QString&);
  void crashed();

private slots:    
  void disconnected();
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  QProcess* mProcess;
  TasSocket* mSocket;
  QString mApplicationName;
  QString mProcessId;
  friend class TasClientManager;
  QTime mCreationTime;
  QDateTime mCrashTime;
#ifdef Q_OS_SYMBIAN
  QString mApplicationUid;
#endif
  QString mPluginType;
};


class ClientRemoveFilter : public ResponseFilter
{
public:
    ClientRemoveFilter(TasCommandModel& model);
   ~ClientRemoveFilter();

	void filterResponse(TasMessage& response);

private:  
	QString mProcessId;
	bool mKill;
	int mWaitTime;
};

#endif
