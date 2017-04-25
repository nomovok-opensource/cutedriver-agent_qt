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

#ifndef SHELLTASK_H
#define SHELLTASK_H

#include <QByteArray>
#include <QString>
#include <QProcess>
#include <QThread>
#include <QMutex>

class ShellTask : public QThread
{
    Q_OBJECT
public:
    enum Status {
        NOT_STARTED,
        RUNNING,
        FINISHED,
        // MINGW did not like the name ERROR?
        ERR,
    };


    ShellTask(const QString& command);
    ~ShellTask();

    void endTask();

    void run();

    ShellTask::Status status();

    qint64 pid() const;
    int returnCode() const;
    QByteArray responseData();

private slots:
    void started();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);

    void readStdOut();
private:
    QString mCommand;
    volatile Status mStatus;
    volatile qint64 mPid;
    int mReturnCode;
    QByteArray mResponse;
    QProcess* mProcess;
    QMutex mutex;
};

#endif
