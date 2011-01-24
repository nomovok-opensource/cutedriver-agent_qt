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
             
#include "startappservice.h"


#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE)) 
#include <windows.h>
#elif (defined(Q_OS_UNIX) || defined(Q_OS_WS_MAC))
#include <unistd.h>
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
    TasLogger::logger()->debug("TasServer::startApplication: " + applicationPath);
    QStringList arguments = args.split(",");

    setRuntimeParams(command);

    if(arguments.contains(SET_PARAMS_ONLY)){
        // do not start app, just need to set the parameters
        response.requester()->sendResponse(response.messageId(), QString("0"));
    }
    else{
        arguments.removeAll(DETACH_MODE);
        arguments.removeAll(NO_WAIT);
        launchDetached(applicationPath, arguments, response);
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


void StartAppService::launchDetached(const QString& applicationPath, const QStringList& arguments, TasResponse& response)
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
        response.setData(QString::number(pid));   
    }
//#elif (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))

//    STARTUPINFO si;
//    PROCESS_INFORMATION pi;

//    ZeroMemory( &si, sizeof(si) );
//    si.cb = sizeof(si);
//    ZeroMemory( &pi, sizeof(pi) );

//    LPTSTR argv = (WCHAR*) ( applicationPath + " -testability " + arguments.join(" ") ).utf16();

//    // Start the child process.
//    if( CreateProcess( NULL,   // No module name (use command line)
//        argv,           // Command line
//        NULL,           // Process handle not inheritable
//        NULL,           // Thread handle not inheritable
//        FALSE,          // Set handle inheritance to FALSE
//        0,              // No creation flags
//        NULL,           // Use parent's environment block
//        NULL,           // Use parent's starting directory
//        &si,            // Pointer to STARTUPINFO structure
//        &pi )           // Pointer to PROCESS_INFORMATION structure
//    )
//    {
//        QString pid = QString::number(pi.dwProcessId);
//        CloseHandle( pi.hProcess );
//        CloseHandle( pi.hThread );

//        TasLogger::logger()->debug("TasServer::launchDetached: application PID " + pid);
//        response.setData(pid);

//    }

//#elif (defined(Q_OS_UNIX) || defined(Q_OS_WS_MAC))



//    pid_t pid;
//    char *const parmList[] =
//    {"/bin/ls", "-l", "/u/userid/dirname", NULL\};
//    char *const envParms[2] = {"STEPLIB=SASC.V6.LINKLIB", NULL\};

//    if ((pid = fork()) ==-1)
//       perror("fork error");
//    else if (pid == 0) {
//       execve("/u/userid/bin/newShell", parmList, envParms);
//       printf("Return not expected. Must be an execve error.\\n");
//    }


#else
    qint64 pid;
    if(QProcess::startDetached(applicationPath, arguments, ".", &pid)){
        response.setData(QString::number(pid));   
    }
#endif
    else{
        TasLogger::logger()->error("TasServer::launchDetached: count not start the application " + applicationPath);
        response.setErrorMessage("Could not start the application " + applicationPath);
    }
}

