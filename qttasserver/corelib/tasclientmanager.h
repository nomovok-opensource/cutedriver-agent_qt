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
#include <QHash>
#include <QList>
#include <QListIterator>
#include <QMutex>
#include <QDateTime>
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

  TasClient* addClient(quint64 processId, const QString& processName=QString());
  TasClient* addRegisteredClient(quint64 processId, const QString& processName, TasSocket* socket,
                                 const QString& type=TAS_PLUGIN);

  void addStartedApp(const QString& processName, qint64 epochString);

  void removeClient(quint64 processId, bool kill=false);
  void removeAllClients(bool kill=true);

  void applicationList(TasObject& parent);
  void startedApplicationsList(TasObject& parent);

  bool writeStartupData(const QString& identifier, const TasSharedData& data);
  bool detachFromStartupData(const QString& identifier);

  TasClient* findClient(TasCommandModel& request);
  TasClient* findByProcessId(quint64 processId);
  TasClient* findByApplicationName(const QString& applicationName);
  TasClient* logMemClient();
  void removeMe(const TasClient& client);

  void removeStartedPid(quint64 pid);
  void addStartedPid(quint64 pid);

private:
  TasClient* latestClient();
  TasClient* removeByProcessId(quint64 processId);

  bool verifyClient(TasClient* client);

private:
  QHash<quint64, TasClient*> mClients;
  QHash<QString, qint64> mStartedApps;
  static TasClientManager* mInstance;
  TasDataShare* mDataShare;
  QMutex mMutex;
  qint32 mMessageCounter;
  QList<quint64> mStartedPids;
};

class TasClient : public QObject
{
  Q_OBJECT
protected:
  TasClient(quint64 processId);
  ~TasClient();

public:
  QString processId();

  const quint64& pid() const;

  void setSocket(TasSocket* socket);
  TasSocket* socket();
  void setApplicationName(const QString& applicationName);
  const QString& applicationName();
  int upTime();
  bool operator ==(const TasClient &client) const;
  void setRegistered();
  void closeConnection();

  void setApplicationUid(const QString& uid);
  QString applicationUid();

  void setPluginType(const QString& pluginType);
  QString pluginType();

  void callFixture(QObject *sender, const char *resultMethod, quintptr callId,
                   const QString &pluginName, const QString &actionName, QHash<QString, QString> parameters);

signals:
  void registered(quint64);

private slots:
  void disconnected();
  void socketDied();

private:
  TasSocket* mSocket;
  QString mApplicationName;
  friend class TasClientManager;
  qint64 mCreationTime;
  QString mApplicationUid;
  QString mPluginType;
  quint64 mPid;
};
#endif
