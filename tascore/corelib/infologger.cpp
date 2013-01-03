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
 

#include <QDir>

#include "taslogger.h"
#include "tasdeviceutils.h"
#include "tascoreutils.h"

#include "infologger.h"

const static QString CPU = "cpu";
const static QString GPU = "gpu";
const static QString MEM = "mem";
const static QString PWR = "pwr";
const static QString ACTION = "action";
const static QString APPEND = "append";
const static QString CLEARLOG = "clearLog";
const static QString FILEPATH = "filePath";

const static QString ATTR_DELIM = ";";
const static QString VALUE_DELIM = ":";

InfoLogger::InfoLogger()
{
    mCpu = 0;
	mMem = 0;
	mGpu = 0;
	mPwr = 0;
    mState = 0;
    mDeviceUtils = new TasDeviceUtils();
    mTimer.setInterval(1000);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(timerEvent()));
}

InfoLogger::~InfoLogger()
{
    mTimer.stop();
    if(mCpu) delete mCpu;
    if(mGpu) delete mGpu;
    if(mMem) delete mMem;
    if(mPwr) delete mPwr;
    delete mDeviceUtils;
}

/*!
  Passes service directed to plugins on to the correct plugin.
*/
void InfoLogger::performLogService(TasCommandModel& model, TasResponse& response)
{
    TasTarget* target = model.findTarget(APPLICATION_TARGET);
    if(!mTimer.isActive() && model.interval() > 100){
        mTimer.setInterval(model.interval());
    }
    if(target){
        TasCommand* command = target->findCommand(CPU);
        if(command){
            if(command->parameter(ACTION) == "start"){
                QString fileName;
                if(!makeFileName(command, CPU, fileName)){
                    response.setErrorMessage("File path must be defined for cpu logging!");
                }
                else{
                    mLastCpuTime = mDeviceUtils->currentProcessCpuTime();
                    mInterval.start();
                    mState |= CpuLogging;     
                    if(mCpu){
                        delete mCpu;
                        mCpu = 0;
                    }
                    mCpu = openFile(fileName, command);
                }
            }
            else if(command->parameter(ACTION) == "stop" || command->parameter(ACTION) == "load"){
                loadCpuData(response, command);
            }
        }
        command = target->findCommand(MEM);
        if(command){
            if(command->parameter(ACTION) == "start"){
                QString fileName;
                if(!makeFileName(command, MEM, fileName)){
                    response.setErrorMessage("File path must be defined for mem logging!");
                }
                else{
                    mState |= MemLogging;
                    if(mMem){
                        delete mMem;
                        mMem = 0;
                    }
                    mMem = openFile(fileName, command);
                }
            }
            else if(command->parameter(ACTION) == "stop" || command->parameter(ACTION) == "load"){
                loadMemData(response, command);
            }

        }
        command = target->findCommand(GPU);
        if(command){
            if(command->parameter(ACTION) == "start"){
                QString fileName;
                if(!makeFileName(command, GPU, fileName)){
                    response.setErrorMessage("File path must be defined for mem logging!");
                }
                else{
                    mState |= GpuLogging;
                    if(mGpu){
                        delete mGpu;
                        mGpu = 0;
                    }
                    mGpu = openFile(fileName, command);
                }

            }
            else if(command->parameter(ACTION) == "stop" || command->parameter(ACTION) == "load"){
                loadGpuData(response, command);
            }

        }
        command = target->findCommand(PWR);
        if(command){
            if(command->parameter(ACTION) == "start"){
                QString fileName;
                if(!makeFileName(command, PWR, fileName)){
                    response.setErrorMessage("File path must be defined for pwr logging!");
                }
                else{
                    mState |= PwrLogging;
                    if(mPwr){
                        delete mPwr;
                        mPwr = 0;
                    }
                    mPwr = openFile(fileName, command);
                }
                //to launch measuring if not running
                mDeviceUtils->pwrDetails();
            }
            else if(command->parameter(ACTION) == "stop" || command->parameter(ACTION) == "load"){
                loadPwrData(response, command);
                if(command->parameter(ACTION) == "stop" ){
                    mDeviceUtils->stopPwrData();
                }
            }

        }
        //determines that does the interval need to be started or stopped
        checkLoggerState();
    }
}

/*!
  Makes the file name for the requested log type. Filename is made from the application
  name plug type and suffix .log. Returns false if the given command does not have 
  the FILEPATH parameter.
*/
bool InfoLogger::makeFileName(TasCommand* command, const QString& type, QString& name)
{
    //cannot make a path since not given
    if(command->parameter(FILEPATH).isEmpty()) return false;

    name = command->parameter(FILEPATH);    

    if(!name.endsWith('/') && !name.endsWith('\\')){
        name.append(QDir::separator());
    }

    name.append(TasCoreUtils::getApplicationName());  
    name.append(type);
    name.append(".log");
    TasLogger::logger()->debug("InfoLogger::makeFileName " + name);
    return true;
}

QFile* InfoLogger::openFile(const QString& fileName, TasCommand* command)
{    
    bool append = (command->parameter(APPEND) == "true");    
    return mLoggerUtil.openFile(fileName, append);
}

/*!
  Loads cpu data from the file and sets the in xml format as
  the response data.
*/
void InfoLogger::loadCpuData(TasResponse& response, TasCommand* command)
{
    if(mCpu){
        response.setData(loadData(mCpu, "cpuLoad", command));    
        if(command->parameter(ACTION) == "stop"){
            delete mCpu;
            mCpu = 0;
            mState ^= CpuLogging;                    
        }
    }
    else{
        response.setErrorMessage("No data collected!");
    }
}

/*!
  Loads mem data from the file and sets the in xml format as
  the response data.
*/
void InfoLogger::loadMemData(TasResponse& response, TasCommand* command)
{
    if(mMem){
        response.setData(loadData(mMem, "memUsage", command));    
        if(command->parameter(ACTION) == "stop"){
            delete mMem;
            mMem = 0;
            mState ^= MemLogging;
        }
    }
    else{
        TasLogger::logger()->debug("InfoLogger::loadMemData no file to load");
        response.setErrorMessage("No data collected!");
    }
}

/*!
  Loads gpu data from the file and sets the in xml format as
  the response data.
*/
void InfoLogger::loadGpuData(TasResponse& response, TasCommand* command)
{
    if(mGpu){
        response.setData(loadData(mGpu, "gpuMemUsage", command));            
        if(command->parameter(ACTION) == "stop"){
            delete mGpu;
            mGpu = 0;
            mState ^= GpuLogging;
        }
    }
    else{
        response.setErrorMessage("No data collected!");
    }
}

/*!
  Loads power data from the file and sets the in xml format as
  the response data.
*/
void InfoLogger::loadPwrData(TasResponse& response, TasCommand* command)
{
    if(mPwr){
        response.setData(loadData(mPwr, "pwrUsage", command));
        if(command->parameter(ACTION) == "stop"){
            delete mPwr;
            mPwr = 0;
            mState ^= PwrLogging;
        }
    }
    else{
        response.setErrorMessage("No data collected!");
    }
}

/*!
  Loads the data from the file and makes a tasdatamodel out of the 
  data. Serializes the data and returns a QByteArray containing the 
  serialized data.
*/
QByteArray InfoLogger::loadData(QFile* file, const QString& name, TasCommand* command)
{
    mTimer.stop();
    QByteArray xmlData = mLoggerUtil.loadLoggedData(file, name);
    if (command->parameter(CLEARLOG) != "false"){
        file->remove();
        if(command->parameter(ACTION) == "load"){
            file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
        }
    }
    return xmlData;
}

/*!
  Starts the logger if not running some data requested to be logged or stops if none.
*/
void InfoLogger::checkLoggerState()
{
    if( (mState & LoggingMask) != 0 ){
        if(!mTimer.isActive()){
            mTimer.start();
        }
    }
    else if( (mState & LoggingMask) == 0){
        mTimer.stop();
    }
}


void InfoLogger::timerEvent()
{
    if( (mState & CpuLogging) != 0){
        logCpu();
    }
    if( (mState & MemLogging) != 0){
        logMem();
    }
    if( (mState & GpuLogging) != 0){
        logGpu();
    }
    if( (mState & PwrLogging) != 0){
        logPwr();
    }
}

void InfoLogger::logMem()
{    
    QString line = "timeStamp:";
    line.append(QDateTime::currentDateTime().toString(DATE_FORMAT));
    line.append(ATTR_DELIM);
    line.append("heapSize");
    line.append(VALUE_DELIM);
    line.append(QString::number(mDeviceUtils->currentProcessHeapSize()));
    mLoggerUtil.writeLine(line, mMem);
}

void InfoLogger::logCpu()
{
    QString line = "timeStamp:";
    line.append(QDateTime::currentDateTime().toString(DATE_FORMAT));
    line.append(ATTR_DELIM);

    line.append("cpuLoad");
    line.append(VALUE_DELIM);

    // Calculate lapsed time in this interval and update time stamp
    int elapsed = mInterval.restart();
    qreal currentCpuTime = mDeviceUtils->currentProcessCpuTime();
    if(currentCpuTime == -1){
        //not supported
        line.append("-1");
    }
    else{
        // Calculate the thread's lapsed CPU time and CPU usage in percentage
        qreal cpuDiff = currentCpuTime - mLastCpuTime;
        mLastCpuTime =  currentCpuTime;
        qreal cpuUsage = ( cpuDiff / elapsed ) * 100;        
        line.append(QString::number(cpuUsage));
    }
    mLoggerUtil.writeLine(line, mCpu);
}

void InfoLogger::logGpu()
{
    GpuMemDetails details = mDeviceUtils->gpuMemDetails();
    QString line = "timeStamp:";
    line.append(QDateTime::currentDateTime().toString(DATE_FORMAT));
    line.append(ATTR_DELIM);
    //not supported
    if(!details.isValid){
        details.totalMem = -1; 
        details.usedMem = -1; 
        details.freeMem = -1; 
        details.processPrivateMem = -1; 
        details.processSharedMem = -1; 
    }
    line.append("totalMem:");
    line.append(QString::number(details.totalMem));
    line.append(ATTR_DELIM);
    line.append("usedMem");
    line.append(VALUE_DELIM);
    line.append(QString::number(details.usedMem));
    line.append(ATTR_DELIM);
    line.append("freeMem");
    line.append(VALUE_DELIM);
    line.append(QString::number(details.freeMem));
    line.append(ATTR_DELIM);
    line.append("processPrivateMem");
    line.append(VALUE_DELIM);
    line.append(QString::number(details.processPrivateMem));
    line.append(ATTR_DELIM);
    line.append("processSharedMem");
    line.append(VALUE_DELIM);
    line.append(QString::number(details.processSharedMem));    
    mLoggerUtil.writeLine(line, mGpu);
}

void InfoLogger::logPwr()
{
    PwrDetails details = mDeviceUtils->pwrDetails();
    QString line = "timeStamp:";
    line.append(QDateTime::currentDateTime().toString(DATE_FORMAT));
    line.append(ATTR_DELIM);
    //not supported
    if(!details.isValid){
        details.voltage = -1;
        details.current = -1;
    }
    line.append("voltage:");
    line.append(QString::number(details.voltage));
    line.append(ATTR_DELIM);
    line.append("current");
    line.append(VALUE_DELIM);
    line.append(QString::number(details.current));
    mLoggerUtil.writeLine(line, mPwr);
}

QByteArray TasInfoLoggerUtil::loadLoggedData(QFile* file, const QString& name )
{
    QHash<QString,QString> params;
    return loadLoggedData(file, name, params);
}


QByteArray TasInfoLoggerUtil::loadLoggedData(QFile* file, const QString& name, QHash<QString,QString> params)
{
    TasDataModel* tasModel = new TasDataModel();
    QString qtVersion = "Qt" + QString(qVersion());
    TasObjectContainer& container = tasModel->addNewObjectContainer(1, qtVersion, "qt");
    TasObject& parentData = container.addNewObject("0", name, "logData");   

    int counter = 0;
    QTextStream in(file);
    in.seek(0);
    while (!in.atEnd()) {
        TasObject& obj = parentData.addObject();
        obj.setId(QString::number(counter));        
        obj.setType("logEntry");
        obj.setName("LogEntry");
        //load attributes format is: title:value;title:value....
        QStringList line = in.readLine().split(ATTR_DELIM, QString::SkipEmptyParts);
        QString attrPair;
        foreach(attrPair, line){
            QStringList attribute = attrPair.split(VALUE_DELIM, QString::SkipEmptyParts);
            obj.addAttribute(attribute.first(),attribute.last());
        }
        counter++;
    }
    parentData.addAttribute("entryCount", counter);   
    if(!params.isEmpty()){
        foreach(QString key, params.keys()){
            parentData.addAttribute(key, params.value(key));   
        }
    }
    QByteArray xml;
    tasModel->serializeModel(xml);
    delete tasModel;
    return xml;
}


void TasInfoLoggerUtil::writeLine(const QString& line, QFile* file)
{
    if(file && file->isWritable()){
        file->write(line.toLatin1());
        file->write("\n");
        file->flush();        
    }
}

QFile* TasInfoLoggerUtil::openFile(const QString& fileName, bool append)
{
    QFile* file = new QFile(fileName);    
    if(append){
        file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);               
    }
    else{
        file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);               
    } 
    return file;
}
