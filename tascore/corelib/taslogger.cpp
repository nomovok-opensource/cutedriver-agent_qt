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
#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>
#include "tasqtcommandmodel.h"
#include "taslogger.h"
#include "tascoreutils.h"

#ifdef Q_OS_SYMBIAN
const char* const C_LOG_PATH = "c:\\logs\\testability\\";
const char* const E_LOG_PATH = "e:\\logs\\testability\\";
#else
const char* const LOG_PATH = "/logs/testability/";
#endif
const int LOG_SIZE = 100000;

void debugOutput(QtMsgType type, const char *msg)
{     
    QString message(msg);
    if (message.length() <= 0)
      return;
    switch (type) 
        {
        case QtDebugMsg:
            TasLogger::logger()->debug(message);
            break;
        case QtWarningMsg:
            TasLogger::logger()->warning(message);
            break;
        case QtCriticalMsg:
            TasLogger::logger()->error(message);
            break;
        case QtFatalMsg:
            TasLogger::logger()->fatal(message);
            break;
        }
}

TasLogger* TasLogger::mInstance = 0;
QMutex* TasLogger::mInstanceMutex = 0;

TasLogger::TasLogger()
{
    mLogSize = LOG_SIZE;
    mEnabled = false;    
    mUseQDebug = false;
    mCurrentLevel = INFO;
    mLogFileName = TasCoreUtils::getApplicationName()+".log";
#ifdef Q_OS_SYMBIAN
    if(QDir(E_LOG_PATH).exists()){
        mLogPath = E_LOG_PATH;
    }
    else{
        mLogPath = C_LOG_PATH;
    }
#else
    mLogPath = LOG_PATH;
#endif
    mOut = 0;
    mEventLogger = new EventLogger();
    mLastLogRollCheck = QTime::currentTime();
}

/*!
  Returns the only instance of the logger.
 */
TasLogger* TasLogger::logger()
{
    if(!mInstance){
        mInstanceMutex = new QMutex();
        mInstance = new TasLogger();        
    }    
	QMutexLocker locker(mInstanceMutex);
    return mInstance;    
}


/*!
  Configures the logger based on the command data. 
  Logger will not check the sanity of the settings.
 */
void TasLogger::configureLogger(TasCommand& command)
{
    //lock the instance 
	QMutexLocker locker(mInstanceMutex);

    if(command.parameter(LOG_LEVEL) == "FATAL"){
        setLevel(FATAL);
    }
    else if(command.parameter(LOG_LEVEL) == "ERROR"){
        setLevel(ERROR);
    }
    else if(command.parameter(LOG_LEVEL) == "INFO"){
        setLevel(INFO);
    }
    else if(command.parameter(LOG_LEVEL) == "WARNING"){
        setLevel(WARNING);
    }
    else if(command.parameter(LOG_LEVEL) == "DEBUG"){
        setLevel(DEBUG);
    }

    if(command.parameter(LOG_TO_QDEBUG) == "true"){
        mUseQDebug = true;
    }
    else if(command.parameter(LOG_TO_QDEBUG) == "false"){
        mUseQDebug = false;
    }

    if(command.parameter(LOG_QDEBUG) == "true"){
        setOutputter(true);
    }
    else if(command.parameter(LOG_QDEBUG) == "false"){
        setOutputter(false);
    }
   
    if(!command.parameter(LOG_FILE_SIZE).isEmpty()){
        mLogSize = command.parameter(LOG_FILE_SIZE).toInt();
    }

    if(command.parameter(CLEAR_LOG) == "true" || !command.parameter(LOG_FOLDER).isEmpty()){
        //need to disable to alter location
        bool wasLogging = mEnabled;
        if(mEnabled) disableLogger();

        if(command.parameter(CLEAR_LOG) == "true"){
            clearLogFile();
        }
        
        if(!command.parameter(LOG_FOLDER).isEmpty()){
            mLogPath = command.parameter(LOG_FOLDER);
            QDir().mkpath(mLogPath);
        }

        //enable the logger unless instructed not to or if it was not on to begin with   
        if( ( (wasLogging && command.parameter(LOG_ENABLE) != "false") || command.parameter(LOG_ENABLE) == "true") && !mUseQDebug){
            enableLogger();        
            debug("TasLogger::configureLogger configuration done and logging enabled.");
        }
    }
    else{
        //enable the logger unless instructed not to or if it was not on to begin with   
        if(command.parameter(LOG_ENABLE) == "true"){
            enableLogger();        
            debug("TasLogger::configureLogger configuration done and logging enabled.");
        }        
        if(command.parameter(LOG_ENABLE) == "false"){
            disableLogger();
        }
    }
}


void TasLogger::configureEventLogger(TasCommand& command)
{
    if(command.parameter(LOG_EVENTS) == "true"){
        QStringList events;
        if(!command.text().isEmpty()){
            events = command.text().split(",");            
        }
        logEvents(events);
    }
    else if(command.parameter(LOG_EVENTS) == "false"){
        stopEventLogging();
    }
}

void TasLogger::setLogSize(int size)
{
    mLogSize = size;
}

TasLogger::~TasLogger()
{
    disableLogger();
    delete mEventLogger;
}

void TasLogger::enableLogger()
{
	QMutexLocker locker(&mMutex);
    mLastLogRollCheck.restart();
    if(!mEnabled && QDir(mLogPath).exists()){
        QString fileName = mLogPath+"/"+mLogFileName;
        mOut = new QFile(fileName);        
        if(mOut->exists(fileName) && mOut->size() > mLogSize){
            QString storageName = mLogPath + "old_" + mLogFileName;
            QFile::remove(storageName);
            QFile::rename(fileName, storageName);
            mOut->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        }
        else{
            mOut->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        }
        mEnabled = true;        
    }    
}

/*!
  Disables the logger.
 */
void TasLogger::disableLogger()
{
	QMutexLocker locker(&mMutex);
    mEnabled = false;
    if(mOut){
        mOut->close();
        delete mOut;
        mOut = 0;
    }
}

void TasLogger::clearLogFile()
{
    if(mOut){
        mOut->remove();
        mOut->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    }
    else{
        QString fileName = mLogPath+"/"+mLogFileName;
        QFile::remove(fileName);
    }
}

void TasLogger::removeLogger()
{
    if(mInstance){
        delete mInstance;
        mInstance = 0;
    }

}


void TasLogger::debug(const QString message)
{
    writeLogLine(DEBUG, message);    
}

void TasLogger::info(const QString message)
{
    writeLogLine(INFO, message);    
}

void TasLogger::error(const QString message)
{
    writeLogLine(ERROR, message);    
}

void TasLogger::warning(const QString message)
{
    writeLogLine(WARNING, message);    
}

void TasLogger::fatal(const QString message)
{
    writeLogLine(FATAL, message);    
}

void TasLogger::setLevel(const LogType& level)
{
    mCurrentLevel = level;
    
}

/*!
  Set the file and path to where to write the logs. The path needs to exists 
  and is not created by the logger. logPath will default to /logs/testability 
  if not set. Will also enable the logger.
 */
void TasLogger::setLogFile(const QString logFileName)
{
    mLogFileName = logFileName;
    disableLogger();
    enableLogger();
}

void TasLogger::setLogDir(const QString logPath)
{
    mLogPath = logPath;
    disableLogger();
    enableLogger();
}

void TasLogger::writeLogLine(LogType type, const QString& message)
{
    if(mEnabled && type <= mCurrentLevel){
      
        if(message.contains("QMetaProperty::read")) return;

        if(!mUseQDebug){
            //check is rolling needed. Done hourly
            if(mLastLogRollCheck.elapsed() > 600000){
                disableLogger();
                enableLogger();
            }
        }
        QString line;
        QTextStream out(&line);

        out << "<";
        out << QTime::currentTime().toString("hh:mm:ss.zzz").toAscii();
        out << ">";
        switch (type) {
        case FATAL:
            out << "FATAL: ";            
            break;            
        case ERROR:
            out << "ERROR: ";            
            break;            
        case INFO:
            out << "INFO: ";            
            break;            
        case WARNING:
            out << "WARNING: ";            
            break;            
        case DEBUG:
            out << "DEBUG: ";    
            break;            
        }
        out << message;
        outPut(line);
    }    
}

void TasLogger::outPut(const QString& line)
{
    if(mUseQDebug){
        qDebug() << line;
    }
    else{
        QMutexLocker locker(&mMutex);
        if(mOut){
            mOut->write(line.toAscii());
            mOut->write("\n");
            mOut->flush();
        }
    }
}

/*!
  Directs all output from this logger to qDebug if true. 
 */
void TasLogger::useQDebug(bool use)
{
    mUseQDebug = use;
    if(mUseQDebug){
        //disable release the log file if there is one
        disableLogger();
        //enable to allow logging (uses qdebug for output)
        mEnabled = true;
    }
    else{
        enableLogger();
    }
}

/*!
  Set to intercept qDebug messages
 */
void TasLogger::setOutputter(bool intercept)
{
    if(intercept){
        qInstallMsgHandler(debugOutput);
    }
    else{
        qInstallMsgHandler(0);
    }
}

void TasLogger::logEvents(QStringList filters)
{
    if(QCoreApplication::instance()){
        debug("TasLogger::logEvents " + filters.join(","));
        mEventLogger->setFilterStrings(filters);
        QCoreApplication::instance()->installEventFilter(mEventLogger);
    }
    else{
        debug("TasLogger::logEvents Cannot set event logging, no qApp!");
    }
}

void TasLogger::stopEventLogging()
{
    if(QCoreApplication::instance()){
        QCoreApplication::instance()->removeEventFilter(mEventLogger);
        mEventLogger->setFilterStrings(QStringList());
    }
}


bool EventLogger::eventFilter(QObject *target, QEvent *event)
{
    QString eventType = TasCoreUtils::eventType(event);
    if(logEvent(eventType)){
        QString objectName = target->objectName();
        QString className  = target->metaObject()->className();
        QString line = "Event type: ";

        QTextStream(&line) <<  TasCoreUtils::eventType(event) << " target class: " << target->metaObject()->className() << " target id: " << TasCoreUtils::objectId(target);

        TasLogger::logger()->debug(line);
    }
    return false;
}    

void EventLogger::setFilterStrings(QStringList filterStrings)
{
    mFilterStrings = filterStrings;
}

bool EventLogger::logEvent(QString eventType)
{
    if(mFilterStrings.size() == 0){
        return true;
    }
    for (int i = 0; i < mFilterStrings.size(); ++i){
        if(eventType.contains(mFilterStrings.at(i))){
            return true;
        }
    }
    return false;
}
