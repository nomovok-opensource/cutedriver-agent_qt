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
#include <QDir>
#include <QDirIterator>

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
#include <errno.h>
#include <sys/wait.h>
#include <QSharedMemory>
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
    QString dir = command.parameter("directory");
    TasLogger::logger()->debug(QString("TasServer::startApplication: '%1'").arg(applicationPath));
    TasLogger::logger()->debug(QString("TasServer::startApplication: Arguments: '%1'").arg(args));
    TasLogger::logger()->debug(QString("TasServer::startApplication: Environment: '%1'").arg(envs));
    TasLogger::logger()->debug(QString("TasServer::startApplication: Directory: '%1'").arg(dir));
    QStringList arguments = args.split(",");
    QStringList environmentVars = envs.split(" ");

    setRuntimeParams(command);

    if(arguments.contains(SET_PARAMS_ONLY)){
        // do not start app, just need to set the parameters
        response.requester()->sendResponse(response.messageId(), QString("0"));
    }
    else{
#if !defined(Q_OS_SYMBIAN)
        //check for search path
        if(!command.parameter("app_path").isEmpty()){
            //look for the app binary
            QString fullName = searchForApp(applicationPath, command.parameter("app_path"));
            if(!fullName.isEmpty()){
                applicationPath = fullName;
            }
        }
#endif
        arguments.removeAll(DETACH_MODE);
        arguments.removeAll(NO_WAIT);
        launchDetached(applicationPath, arguments, environmentVars, dir, response);
        //add pids to startedapp pid list
        if(!response.isError() && !response.dataAsString().isEmpty() ){
            TasClientManager::instance()->addStartedPid(response.dataAsString());
        }
    }
}

QString StartAppService::searchForApp(const QString& appName, const QString& rootPath)
{
    TasLogger::logger()->debug("StartAppService::searchForApp " + appName + " from " + rootPath);
    QFileInfo filename(appName);
    //check that if the file exists
    if( filename.exists()){
        TasLogger::logger()->debug("StartAppService::searchForApp given name exists return it " + filename.absoluteFilePath());
        return filename.absoluteFilePath();
    }
    QDir root(rootPath);
    if(!root.exists()){
        TasLogger::logger()->debug("StartAppService::searchForApp given root path does not exist.");
        return QString();
    }

    QStringList apps;
    apps << filename.baseName();
    apps << filename.baseName() + ".exe";

    QDirIterator iter(rootPath, apps, QDir::Files | QDir::Executable,
                      QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    //if more than one found return the first one
    QString applicationPath;
    while(iter.hasNext()){
        QString name = iter.next();
        if(name.isEmpty()){
            break;
        }
        QFileInfo match(name);
        if(match.isExecutable()){
            applicationPath = match.filePath();
            TasLogger::logger()->debug("StartAppService::searchForApp match found: " + applicationPath);
            break;
        }
        else{
            TasLogger::logger()->debug("StartAppService::searchForApp match not executable");
        }
    }
    return applicationPath;
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


void StartAppService::launchDetached(const QString& applicationPath, const QStringList& arguments,
                                     const QStringList& environmentVars, const QString &workingDirectory,
                                     TasResponse& response)
{
    QString additionalMessage;
#ifdef Q_OS_SYMBIAN
    //Qt startDetach seems to leak memory so need to do it for now.
    //to be removed when fix in qt

    if (!workingDirectory.isEmpty()) {
        TasLogger::logger()->warning(QString("TasServer::launchDetached: Working directory not supported on Symbian"));
    }

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
    }
#elif (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Arguments
    QString argv = applicationPath + " " + arguments.join(" ");

    // Environment bloc variable
    QStringList envList = QProcess::systemEnvironment() << environmentVars;
    WCHAR* envp = (WCHAR*)malloc((envList.join(" ").length() + 2) * sizeof(WCHAR)); // just counting memory in cluding the NULL (\0) string ends and binal NULL block end
    LPTSTR env = (LPTSTR) envp;
    LPCTSTR lpCurrentDirectory  = NULL;

    if (!workingDirectory.isEmpty()) {
        lpCurrentDirectory  = (LPCTSTR)workingDirectory.utf16();
    }
    // else use current working directory

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
        lpCurrentDirectory ,        // Working directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        QString pid = QString::number(pi.dwProcessId);
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
        TasLogger::logger()->debug( QString("TasServer::launchDetached: Child PID: %1").arg(pid) );
        TasClientManager::instance()->addStartedApp(applicationPath, QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        response.setData(pid);

    }

#elif (defined(Q_OS_UNIX) || defined(Q_OS_WS_MAC))

    pid_t pid, sid, grandpid;

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
    char **envListArray = new char*[ envList.length() + 1 ];
    for( int i = 0; i < envList.length(); i++)
    {
        QByteArray variable = ((QString) envList[i]).toLocal8Bit();
        char *const variablePtr = new char[variable.length() + 1];
        strcpy(variablePtr, variable.data());
        envListArray[i] = variablePtr;
    }
    envListArray[envList.length()] = NULL;


    // Try to find the binary first
    QString filePath("");
    const QString path = QString::fromLocal8Bit(::getenv("PATH"));
    const QString file = QString::fromLocal8Bit(paramListArray[0]);
    if (!file.contains('/') && !path.isEmpty()) 
    {
        QStringList pathEntries = path.split(QLatin1Char(':'));
        foreach(QString dir, pathEntries) 
        {
            if (!dir.endsWith(QDir::separator())) {
                dir += QDir::separator();
            }
            filePath = dir + file;
            if (QFile::exists(filePath)) {
                break;
            } else {
                filePath = "";
            }
        }
    } else if (QFile::exists(file)) {
        filePath= file;
    }

    if (filePath.isEmpty()) {
        additionalMessage = ". The file was not found or is not in PATH.";
    }

    // Using shared memory pro IPC instead of qt pipes to avoid using private qt libraries.
    QSharedMemory mem("pid_mem");
    if ( !filePath.isEmpty() && mem.create(sizeof(pid_t)) )
    {

        mem.lock();
        pid_t *mem_ptr = (pid_t *) mem.data();
        *mem_ptr = 0;
        mem.unlock();

        // START MAKING CHILDREN HERE :D
        // Child
        if ( (pid = fork()) == 0)
        {
            // Create new session for the process (detatch from parent process group)
            sid = setsid();
            if ( sid < 0 )
            {
                TasLogger::logger()->error( QString("TasServer::launchDetached: Failed to detach child."));
                _exit(1);
            }

            // Grandchild
            if ( ( grandpid = fork() ) == 0 )
            {
                // detach ont he grandchild.
                while (mem.isAttached())
                {
                    bool res = mem.detach(); // can fail if mem is locked
                    if (!res) TasLogger::logger()->error("TasServer::launchDetached: GrandChild::Shared Memory Detachment failed! Retrying.");
                    if (mem.isAttached()) Sleeper::sleep(100);
                }

                // Change working directory
                if (!workingDirectory.isEmpty()) {
                    if (chdir(workingDirectory.toLocal8Bit().constData()) == -1) {
                        TasLogger::logger()->error( QString("TasServer::launchDetached: chdir(\"%1\") error: %2")
                                                   .arg(workingDirectory).arg(strerror(errno)) );
                    }
                }

                // Try see if we don't need path
                paramListArray[0] = QFile::encodeName(filePath).data();
                execve( paramListArray[0], paramListArray, envListArray);
                TasLogger::logger()->error( QString("TasServer::launchDetached: Granhild process died straight away."));
                _exit(1);
            }

            // Child send Grandchild pid to shared mem and exit in order to end detachment of grandchild
            else if( grandpid > 0)
            {
                mem.lock();
                pid_t *mem_ptr = (pid_t *) mem.data();
                *mem_ptr = grandpid;
                mem.unlock();
                while (mem.isAttached())
                {
                    bool res = mem.detach();
                    if (!res) TasLogger::logger()->error("TasServer::launchDetached:Child::Shared Memory Detachment failed! Retrying.");
                    if (mem.isAttached()) Sleeper::sleep(100);
                }
                _exit(0);
            }
            else
            {
                // if child fork fails detach mem and kill child
                // TODO Return with error?
                while (mem.isAttached())
                {
                    bool res = mem.detach(); // can fail if mem is locked
                    if (!res) TasLogger::logger()->error("TasServer::launchDetached: Child::SecondForkError::Shared Memory Detachment failed! Retrying.");
                    if (mem.isAttached()) Sleeper::sleep(100);
                }
                TasLogger::logger()->error( QString("TasServer::launchDetached: Failed second fork().Could not start the application " + applicationPath));
                _exit(1);
            }

        }

        // Parent
        else if (pid > 0) {

            pid_t actualpid = 0;
            int count = 0;
            while(actualpid == 0 && count < 10)
            {
                mem.lock();
                pid_t *mem_ptr = (pid_t *) mem.data();
                actualpid = *mem_ptr;
                mem.unlock();
                TasLogger::logger()->debug( QString("TasServer::launchDetached: ACTUAL Child PID: %1").arg((int)actualpid) );
                if (actualpid == 0 )
                {
                    Sleeper::sleep(100);
                    count++;
                }
            }
            while (mem.isAttached())
            {
                bool res = mem.detach(); // can fail if mem is locked
                if (!res) TasLogger::logger()->error("TasServer::launchDetached: Parent::Shared Memory Detachment failed! Retrying.");
                if (mem.isAttached()) Sleeper::sleep(100);
            }

            // Reaping zomby child
            int childreturn;
            waitpid(pid, &childreturn, 0);

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

            if (actualpid == 0)
            {
                TasLogger::logger()->error("TasServer::launchDetached: Error getting the PID for the new application " + applicationPath);
                response.setErrorMessage("Could not start the application " + applicationPath);
            }
            else if (childreturn != 0)
            {
                TasLogger::logger()->error("TasServer::launchDetached: Error waiting for zombie child " + applicationPath);
                response.setErrorMessage("Could not start the application " + applicationPath);
            }
            else
            {
                TasLogger::logger()->debug( QString("TasServer::launchDetached: Child PID: %1").arg((int)actualpid) );
                response.setData(QString::number((int) actualpid));
            }
        }
        else
        {
            //fails, clear mem and send error
            while (mem.isAttached())
            {
                bool res = mem.detach();
                if (!res) TasLogger::logger()->error("TasServer::launchDetached: Parent::ForkError::Shared Memory Detachment failed! Retrying.");
                if (mem.isAttached()) Sleeper::sleep(100);
            }
            TasLogger::logger()->error("TasServer::launchDetached: Failed first fork(). Could not start the application " + applicationPath);
            response.setErrorMessage("Could not start the application " + applicationPath);

        }
    }


#else
    qint64 pid;
    if(QProcess::startDetached(applicationPath, arguments, ".", &pid)){

            TasClientManager::instance()->addStartedApp(applicationPath, QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
        response.setData(QString::number(pid));
    }
#endif
    else{
#if (defined(Q_OS_UNIX) || defined(Q_OS_WS_MAC)) && !defined(Q_OS_SYMBIAN)
        // if parent fork fails, clear mem and send error
        while (mem.isAttached())
        {
            bool res = mem.detach(); // can fail if mem is locked
            if (!res) TasLogger::logger()->error("TasServer::launchDetached: Parent::SharedMemErorr:: Shared Memory Dtachement failed! Retrying.");
            if (mem.isAttached()) Sleeper::sleep(100);
        }
#endif
        TasLogger::logger()->error("TasServer::launchDetached: Could not start the application " + applicationPath);
        response.setErrorMessage("Could not start the application " + applicationPath + additionalMessage);
    }
#if (defined(Q_OS_WIN32) || defined(Q_OS_WINCE))
        free(envp);
#endif
}

