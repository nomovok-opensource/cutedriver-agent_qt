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





#include "shelltask.h"


 


#include <taslogger.h>
#include <QProcess>
#include <QMutexLocker>
#include <QMetaType>

#ifdef Q_OS_WIN32
#include <Windows.h>
#endif



ShellTask::ShellTask(const QString &command) : 
    mCommand(command), 
    mStatus(ShellTask::NOT_STARTED), 
    mPid(0),
    mProcess(0)
{
}

ShellTask::~ShellTask()
{
    if (mProcess) {
        if (mProcess->state() == QProcess::Running ||
            mProcess->state() == QProcess::Starting) {
            mProcess->terminate();
            //mProcess->kill();
        }
        mProcess->deleteLater();
        mProcess=0;
    }            
}

void ShellTask::run()
{
    TasLogger::logger()->debug("ShellTask::run task");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    mProcess = new QProcess(this);
    connect(mProcess, SIGNAL(started()), 
            this, SLOT(started()));
    connect(mProcess, SIGNAL(error(QProcess::ProcessError)) ,
            this, SLOT(processError(QProcess::ProcessError)));

    connect(mProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(mProcess, SIGNAL(readyReadStandardOutput()), 
            this, SLOT(readStdOut()));

    mProcess->setReadChannelMode(QProcess::MergedChannels);
    mProcess->setEnvironment(QProcess::systemEnvironment());

    mProcess->start(mCommand);
#ifdef Q_OS_WIN32
    mPid = mProcess->pid()->dwProcessId;
#else
	mPid = mProcess->pid();
#endif
    mProcess->closeWriteChannel();

    exec();
}

ShellTask::Status ShellTask::status() 
{
    QMutexLocker locker(&mutex);
    return mStatus;
}

QByteArray ShellTask::responseData()
{
    QMutexLocker locker(&mutex);
    QByteArray response = mResponse;
    mResponse.clear();
    return response;
}

qint64 ShellTask::pid() const
{
    return mPid;
}

int ShellTask::returnCode() const
{
    if (mProcess) {
        return mProcess->exitCode();
    } 
    return 0;
}

void ShellTask::readStdOut()
{
    QMutexLocker locker(&mutex);
    mResponse.append(mProcess->readAll());    
}

void ShellTask::started()
{
    TasLogger::logger()->debug("ShellTask::started");
    mStatus = ShellTask::RUNNING;
}
void ShellTask::finished(int exitCode, QProcess::ExitStatus )
{
    QMutexLocker locker(&mutex);
    TasLogger::logger()->debug("ShellTask::finished");
    mResponse.append(mProcess->readAll());
    mStatus = ShellTask::FINISHED;
    mReturnCode = exitCode;

}

void ShellTask::processError(QProcess::ProcessError processError) 
{
    TasLogger::logger()->debug("ShellTask::error in task");
	
    mStatus = ShellTask::ERR;
    switch (processError) {
    case QProcess::FailedToStart:
        mResponse = QString("Command failed to start").toUtf8();

        break;
    case QProcess::Crashed:
        mResponse = QString("Program crashed.").toUtf8();
        break;
    case QProcess::Timedout:
        mResponse = QString("Timeout occurred.").toUtf8();
        break;
    case QProcess::WriteError:
        mResponse = QString("Write error.").toUtf8();
        break;
    case QProcess::ReadError:
        mResponse = QString("Read error.").toUtf8();
        break;
    case QProcess::UnknownError:
    default:
        mResponse = QString("Unknown error.").toUtf8();
        break;
    }

}
