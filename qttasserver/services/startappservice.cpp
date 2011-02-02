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
 
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>

#include <taslogger.h>
#include <tascoreutils.h>
#include <tasdatashare.h>

#include "tasdeviceutils.h"
#include "tasclientmanager.h"
             
#include "startappservice.h"


#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE)) 
#include <windows.h>

#elif (defined(Q_OS_UNIX) || defined(Q_OS_WS_MAC))
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "private/qcore_unix_p.h"
#endif

const char* const SET_PARAMS_ONLY = "set_params_only";
const char* const DETACH_MODE = "detached";
const char* const NO_WAIT = "noWait";



StartAppService::StartAppService()
{}

StartAppService::~StartAppService()
{}

bool StartAppService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        // Turn screen on.
        TasDeviceUtils::resetInactivity();

        TasCommand* command = getCommandParameters(model, "Run");
        if(command){
            startApplication(*command, response);
        }
        else{
            TasLogger::logger()->error("StartAppService::executeService no Run command found!");
            response.setErrorMessage("Could not parse Run command from the request!");
        }
        return true;
    }
    else{
        return false;
    }
}

/*!
  Attempts to start a process using the application path send in the command model.
 */
void StartAppService::startApplication(TasCommand& command, TasResponse& response)
{
    QString applicationPath = command.parameter("application_path");    
    QString args = command.parameter("arguments");
    QString envs = command.parameter("environment");
    TasLogger::logger()->debug(QString("TasServer::startApplication: '%1'").arg(applicationPath));
    TasLogger::logger()->debug(QString("TasServer::startApplication: Arguments: '%1'").arg(args));
    TasLogger::logger()->debug(QString("TasServer::startApplication: Environment: '%1'").arg(envs));
    QStringList arguments = args.split(",");
    QStringList environmentVars = envs.split(" ");


    setRuntimeParams(command);

    if(arguments.contains(SET_PARAMS_ONLY)){
        // do not start app, just need to set the parameters
        response.requester()->sendResponse(response.messageId(), QString("0"));
    }
    else{
        arguments.removeAll(DETACH_MODE);
        arguments.removeAll(NO_WAIT);
        launchDetached(applicationPath, arguments, environmentVars, response);
    }
}


void StartAppService::setRuntimeParams(TasCommand& command)
{
    QString applicationPath = command.parameter("application_path");    
    QString eventList = command.parameter("events_to_listen");
    QString signalList = command.parameter("signals_to_listen");    
    TasLogger::logger()->debug("StartAppService::setRuntimeParams signals: " + signalList);
    if(!eventList.isEmpty() || !signalList.isEmpty()){
        TasSharedData startupData(eventList.split(","), signalList.split(","));
        QString identifier = TasCoreUtils::parseExecutable(applicationPath);
        if(!TasClientManager::instance()->writeStartupData(identifier, startupData)){
            TasLogger::logger()->error("StartAppService::setRuntimeParams could not set run time params for identifier: " + identifier + "!");
        }
        else {
            TasLogger::logger()->error("StartAppService::setRuntimeParams set with identifier: " + identifier);
        }
    }
}


QHash<QString, QString> StartAppService::parseEnvironmentVariables(const QString& env) {
    QHash<QString,QString> vars;
    QStringList var = env.split(" ");
    foreach(QString str, var) {
        QStringList key = str.split("=");
        if (key.size() == 2) {
            vars[key.at(0)] = key.at(1);
        }
    }
    return vars;
}


#ifdef Q_OS_SYMBIAN 
//Qt startDetach seems to leak memory so need to do it for now.
//to be removed when fix in qt
static void qt_create_symbian_commandline(
    const QStringList &arguments, const QString &nativeArguments, QString &commandLine)
{
    for (int i = 0; i < arguments.size(); ++i) {
        QString tmp = arguments.at(i);
        tmp.replace(QLatin1String("\\\""), QLatin1String("\\\\\""));
        tmp.replace(QLatin1String("\""), QLatin1String("\\\""));
        if (tmp.isEmpty() || tmp.contains(QLatin1Char(' ')) || tmp.contains(QLatin1Char('\t'))) {
            QString endQuote(QLatin1String("\""));
            int i = tmp.length();
            while (i > 0 && tmp.at(i - 1) == QLatin1Char('\\')) {
                --i;
                endQuote += QLatin1String("\\");
            }
            commandLine += QLatin1String("\"") + tmp.left(i) + endQuote + QLatin1Char(' ');
        } else {
            commandLine += tmp + QLatin1Char(' ');
        }
    }

    if (!nativeArguments.isEmpty())
        commandLine += nativeArguments;
    else if (!commandLine.isEmpty()) // Chop the extra trailing space if any arguments were appended
        commandLine.chop(1);
}
#endif


void StartAppService::launchDetached(const QString& applicationPath, const QStringList& arguments, const QStringList& environmentVars, TasResponse& response)
{

#ifdef Q_OS_SYMBIAN 
//Qt startDetach seems to leak memory so need to do it for now.
//to be removed when fix in qt
    qint64 pid;
    QString commandLine;
    QString nativeArguments;
    qt_create_symbian_commandline(arguments, nativeArguments, commandLine);
    TPtrC program_ptr(reinterpret_cast<const TText*>(applicationPath.constData()));
    TPtrC cmdline_ptr(reinterpret_cast<const TText*>(commandLine.constData()));
    RProcess process;
    if( process.Create(program_ptr, cmdline_ptr) == KErrNone){
        process.Resume();
        pid = process.Id().Id();
        process.Close();
        TasClientManager::instance()->addStartedApp(applicationPath, QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        response.setData(QString::number(pid));   
    }require 'tdriver'
@sut = TDriver.sut(:Id => 'sut_qt')
@app = @sut.run(:name => '/usr/bin/calculator')

#elif (defined(Q_OS_WIN32) && defined(Q_OS_WINCE) && defined(Q_OS_UNIX)) //ignore untill fixed

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Arguments
    QString argv = applicationPath + arguments.join(" ");

    // Environment bloc variable
    QStringList envList = QProcess::systemEnvironment() << environmentVars;

    WCHAR envp[envList.join(" ").length() + 2]; // just counting memory in cluding the NULL (\0) string ends and binal NULL block end
    LPTSTR env = (LPTSTR) envp;

    for (int i = 0; i < envList.length(); i++)
    {
        env = lstrcpy( env, (LPTSTR) envList[i].utf16() );
        env += lstrlen( (LPTSTR) envList[i].utf16() ) +1;
    }
    *env = (WCHAR) NULL;

    // DEBUG
//    LPTSTR lpszVariable = envp;
//    while (*lpszVariable) //while not null
//    {
//        TasLogger::logger()->debug( QString("TasServer::launchDetached: ENV: %1").arg(QString::fromUtf16((ushort *) lpszVariable)) );
//        lpszVariable += lstrlen(lpszVariable) + 1;
//    }



    // Start the child process.
    if( CreateProcess( NULL,   // No module name (use command line)
        (WCHAR *) argv.utf16(),           // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_UNICODE_ENVIRONMENT,              // 0 for no creation flags
        (LPVOID) envp,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        QString pid = QString::number(pi.dwProcessId);
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
        TasLogger::logger()->debug( QString("TasServer::launchDetached: Child PID: %1").arg(pid) );
        response.setData(pid);

    }

#elif (defined(Q_OS_UNIX) && defined(Q_OS_WS_MAC) && defined(Q_OS_WIN32) )

    pid_t pid, sid, grandpid;

    int pidPipeDesc[2];
    qt_safe_pipe(pidPipeDesc);

    // Create Arguments ARRAY (application path to executable on first element)
    QStringList paramList;
    paramList << applicationPath;
    paramList << arguments;
    char **paramListArray = new char*[ paramList.length() + 1 ];
    for( int i = 0; i < paramList.length(); i++)
    {
        QByteArray variable = ((QString) paramList[i]).toLocal8Bit();
        char *const variablePtr = new char[variable.length() + 1];
        strcpy(variablePtr, variable.data());
        paramListArray[i] = variablePtr;
    }
     paramListArray[paramList.length()] = NULL;


    // Create environment Array with NULL end element
    QStringList envList = QProcess::systemEnvironment() << environmentVars;
    //TasLogger::logger()->debug(QString("TasServer::startApplication: ALL '%1'").arg(envList.join(",")));
    //TasLogger::logger()->debug(QString("TasServer::startApplication: USER '%1'").arg(environmentVars.join(",")));

    char **envListArray = new char*[ envList.length() + 1 ];
    for( int i = 0; i < envList.length(); i++)
    {
        QByteArray variable = ((QString) envList[i]).toLocal8Bit();
        char *const variablePtr = new char[variable.length() + 1];
        strcpy(variablePtr, variable.data());
        envListArray[i] = variablePtr;
    }
    envListArray[envList.length()] = NULL;

    // START MAKING CHILDREN HERE :D
    // Child
    if ( (pid = fork()) == 0) {

        // We are only going to write on the pipe for papa
        qt_safe_close(pidPipeDesc[0]);

        // Create new session for the process (detatch from parent process group)
        sid = setsid();
        if ( sid < 0 )
        {
            TasLogger::logger()->error( QString("TasServer::launchDetached:Failed to detach child."));
            exit(1);
        }

        // Grandchild
        if ( ( grandpid = fork() ) == 0 )
        {
            // Try see if we don't need path
            execve( paramListArray[0], paramListArray, envListArray);

            // Try also on all path directories if above fails
            const QString path = QString::fromLocal8Bit(::getenv("PATH"));
            const QString file = QString::fromLocal8Bit(paramListArray[0]);
            if (!path.isEmpty())
            {
                QStringList pathEntries = path.split(QLatin1Char(':'));
                for (int k = 0; k < pathEntries.size(); ++k) {
                    QByteArray tmp = QFile::encodeName(pathEntries.at(k));
                    if (!tmp.endsWith('/')) tmp += '/';
                    tmp += QFile::encodeName(file);
                    paramListArray[0] = tmp.data();
                    TasLogger::logger()->error( QString("TasServer::launchDetached: PATH = '%1'").arg((char *) paramListArray[0]));
                    execve( paramListArray[0], paramListArray, envListArray);

                }
             }

            TasLogger::logger()->error( QString("TasServer::launchDetached: Granhild process died straight away."));
        }

        // Child exit in order to end detachment of grandchild
        else if( grandpid > 0)
        {
            qt_safe_write(pidPipeDesc[1], &grandpid, sizeof(pid_t));
            qt_safe_close(pidPipeDesc[1]);
            _exit(0);
        }

    }

    // Parent
    else if (pid > 0) {

        // We are only going to read from the pipe from child
        qt_safe_close(pidPipeDesc[1]);
        pid_t actualpid = 0;
        qt_safe_read(pidPipeDesc[0], &actualpid, sizeof(pid_t));
        qt_safe_close(pidPipeDesc[0]);
        pid = actualpid;

        // Free memory
        for (int i = 0; i < paramList.length(); i++ )
        {
            delete [] paramListArray[i];
        }
        delete [] paramListArray;

        for (int i = 0; i < envList.length(); i++ )
        {
            delete [] envListArray[i];
        }
        delete [] envListArray;

        TasLogger::logger()->debug( QString("TasServer::launchDetached: Child PID: %1").arg((int)pid) );
        response.setData(QString::number((int) pid));
    }


#else
    qint64 pid;
    if(QProcess::startDetached(applicationPath, arguments, ".", &pid)){

	    TasClientManager::instance()->addStartedApp(applicationPath, QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        response.setData(QString::number(pid));   
    }
#endif
    else{
        TasLogger::logger()->error("TasServer::launchDetached: Could not start the application " + applicationPath);
        response.setErrorMessage("Could not start the application " + applicationPath);
    }
}

