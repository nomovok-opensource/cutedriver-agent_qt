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
 

#include <tasqtdatamodel.h>
#include <taslogger.h>

#include "shelltask.h"
#include <QThreadPool>
#include "shellcommandservice.h"


ShellCommandService::ShellCommandService()
{
}

ShellCommandService::~ShellCommandService()
{
}

bool ShellCommandService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        TasCommand* command = getCommandParameters(model, "shellCommand");
        if(command && !command->text().isEmpty()){
            if (command->parameter("detached") == "true"){
                detachedShellCommand(command->text(), response);
            }
            else if (command->parameter("threaded") == "true") {
                shellTask(command->text(), response);
            }
            else if (command->parameter("status") == "true") {
                TasCommand* command = getCommandParameters(model, "shellCommand");
                if(command && !command->text().isEmpty()){
                    qint64 pid = command->text().toInt();
                    if (command->parameter("kill") == "true") {
                        killTask(pid, response);
                    } else {
                        shellStatus(pid, response);
                    }
                }
            }
            else{
                shellCommand(command->text(), response);
            }
        }
        else{
            response.setErrorMessage(NO_COMMAND_TO_EXECUTE);
        }
        return true;
    }
    else{
        return false;
    }
}

void ShellCommandService::killTask(qint64 pid, TasResponse& response)
{
    ShellTask* task = mTasks.value(pid);
    if (task) {
        TasLogger::logger()->debug("ShellCommandService::terminating task");
        task->endTask(); // Deleteafter in terminate
        mTasks.remove(pid);
        TasLogger::logger()->debug("ShellCommandService::killTask Pid " + QString::number(pid) + " was killed.");
        response.setData("Pid " + QString::number(pid) + " was killed.");
    } else {
        TasLogger::logger()->debug("ShellCommandService::killTask Pid " + QString::number(pid) + " was not found.");
        response.setData("Pid " + QString::number(pid) + " was not found.");
    }
    
}

void ShellCommandService::shellStatus(qint64 pid, TasResponse& response)
{
    TasLogger::logger()->debug("ShellCommandService::service: looking for pid " + 
                               QString::number(pid));
    
    if (mTasks.contains(pid)) {
        TasLogger::logger()->debug("ShellCommandService::got it");

        ShellTask* task = mTasks.value(pid);

        TasLogger::logger()->debug("ShellCommandService::setting run data");
        TasDataModel* model = new TasDataModel();
        QString qtVersion = "Qt" + QString(qVersion());
        TasObjectContainer& container = model->addNewObjectContainer(1, qtVersion, "qt");
        
        TasObject& output = container.addNewObject("2","Response","Response");
        
        ShellTask::Status status = task->status();

        switch (status) {
        case ShellTask::ERR:
            output.addAttribute("status", "ERROR");
            break;            
        case ShellTask::RUNNING:
            output.addAttribute("status", "RUNNING");
            break;
        case ShellTask::FINISHED:
            output.addAttribute("status", "FINISHED");
            output.addAttribute("exitCode", task->returnCode());
            break;            
        case ShellTask::NOT_STARTED:
        default:
            output.addAttribute("status", "NOT_STARTED");
            break;            

        }        
        output.addAttribute("output", QString(task->responseData()));    
    
        // Clean up if process is done.
        if (status != ShellTask::RUNNING) {            
            mTasks.remove(pid);
            if (task->isRunning()) {
                task->endTask();
            } else {
                task->deleteLater();
                task = 0;
            }


            TasLogger::logger()->debug("ShellCommandService::service: deleting");
	
            TasLogger::logger()->debug("ShellCommandService::service: donne");
        }
        
        QByteArray* xml = new QByteArray();
        model->serializeModel(*xml);
        delete model;
        response.setData(xml);
    }

    TasLogger::logger()->debug("ShellCommandService::service: shell status done");
}

/*!

Executes a command as a process
Output written to given socket as response.
*/
void ShellCommandService::shellCommand(QString message, TasResponse& response)
{
    TasLogger::logger()->debug("ShellCommandService::shellCommand: " + message);

    QProcess process;
    process.setReadChannelMode(QProcess::MergedChannels);
    process.setEnvironment(QProcess::systemEnvironment());
    process.start(message);
    
    process.closeWriteChannel();
    process.waitForFinished(4000);

    QByteArray output = process.readAll();
    response.setData(output);
}

/*!

Executes a command as a process
Output written to given socket as response.
*/
void ShellCommandService::detachedShellCommand(QString message, TasResponse& response)
{
    TasLogger::logger()->debug("ShellCommandService::detachedShellCommand: " + message);
    QProcess process;
    process.setReadChannelMode(QProcess::MergedChannels);
    process.setEnvironment(QProcess::systemEnvironment());
    bool started = process.startDetached(message);

    if(!started){
        response.setErrorMessage("Failed to start process!");
    }
}


void ShellCommandService::finished()
{
    TasLogger::logger()->debug("ShellCommandService::shellTask: ending threaded task");
    sender()->deleteLater();
}

void ShellCommandService::shellTask(const QString&  command, TasResponse &response)
{
    TasLogger::logger()->debug("ShellCommandService::shellTask: " + command);

    ShellTask* task = new ShellTask(command);
    connect(task, SIGNAL(finished()), this, SLOT(finished()));
    task->start();
    // Wait for the thread to start.
    TasCoreUtils::wait(1000);
    qint64 pid = task->pid();
    if (pid != 0) {
        mTasks[task->pid()] = task;
    } else {
        delete task;
    }
    response.setData(QString::number(pid));
}
