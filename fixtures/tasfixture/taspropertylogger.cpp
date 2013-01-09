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
 

#include <QObject>
#include <QDir>


#include <tascoreutils.h>
#include "taspropertylogger.h"

const char* const PROPERTY = "property";
const char* const APPEND = "append";
const char* const CLEARLOG = "clearLog";
const char* const FILEPATH = "filePath";
const char* const ATTR_DELIM = ";";
const char* const VALUE_DELIM = ":";
const char* const INTERVAL = "interval";


TasPropertyLogger::TasPropertyLogger()
{
    mInterval = 1000;
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(logTimerEvent()));
}
TasPropertyLogger::~TasPropertyLogger()
{
    mTimer.stop();
    foreach( QObject* key, mTargets.keys()){
        qDeleteAll(mTargets.value(key));
    }
    mTargets.clear();

}

/*!
  Collect the property values
 */
void TasPropertyLogger::logTimerEvent()
{
    foreach( QObject* object, mTargets.keys()){
        QHash<QString,QFile*> fileMap = mTargets.value(object);
        foreach(QString property, fileMap.keys()){
            QString line = "timeStamp:";
            line.append(QDateTime::currentDateTime().toString(DATE_FORMAT));
            line.append(ATTR_DELIM);
            line.append(property);
            line.append(VALUE_DELIM);
            line.append(object->property(property.toLatin1()).toString());
            mLoggerUtil.writeLine(line, fileMap.value(property));
        }
    }
}

/*!
  Starts logging property values if possible. Returns false if logging cannot be started.
  Check error message in such cases.
 */
bool TasPropertyLogger::startPropertyLog(QObject* object, QHash<QString, QString> params, QString &errorMsg)
{
    if(!validateParams(params, errorMsg)){
        return false;
    }
    
    QString property = params.value(PROPERTY);
    if(!object->property(property.toLatin1()).isValid()){
        errorMsg = "Target object does not have a property "+property+".";
        return false;
    }
    if(!params.contains(FILEPATH)){
        errorMsg = "Root directory for the log data must be defined";
        return false;
    }

    QHash<QString,QFile*> fileMap;
    if(mTargets.contains(object)){
        fileMap = mTargets.value(object);
    }
    if(!fileMap.contains(property)){
        QString name = params.value(FILEPATH);    
        if(!name.endsWith('/') && !name.endsWith('\\')){
            name.append(QDir::separator());
        }
        name.append(TasCoreUtils::getApplicationName());  
        name.append("_");
        name.append(object->metaObject()->className());
        name.append("_");
        name.append(property);
        name.append(".log");
        bool append = (params.value(APPEND, "") == "true");    
        QFile* file = mLoggerUtil.openFile(name, append);
        if(file->error()!= QFile::NoError){
            errorMsg = "Opening file failed, reason: " + QString::number(file->error());
            delete file;
            return false;
        }
        fileMap.insert(property, file);
    }
    mTargets.insert(object, fileMap);
    
    if(params.contains(INTERVAL)){
        int interval = params.value(INTERVAL).toInt();
        if(interval != mInterval){
            mInterval = interval;
            mTimer.stop();
        }
    }
    if(!mTimer.isActive()){
        mTimer.start(mInterval);
    }
    return true;
}


bool TasPropertyLogger::getLogData(QObject* object, QHash<QString, QString> params, QString& data)
{
    if(!mTargets.contains(object)){
        data = QString("No data collected for the object.").toUtf8();
        return false;
    }
    else{
        QString errorMsg;
        if(!validateParams(params, errorMsg)){
            data = errorMsg;
            return false;
        }
        QString property = params.value(PROPERTY);
        QHash<QString,QFile*> fileMap = mTargets.value(object);
        if(!fileMap.contains(property)){
            data = QString("No data collected for the property: " + property);
            return false;
        }
        else{
            QFile* file = fileMap.value(property);
            QHash<QString,QString> objectAttrs;
            objectAttrs.insert("objectType",object->metaObject()->className());
            objectAttrs.insert("objectName",object->objectName());
            data = QString::fromUtf8(mLoggerUtil.loadLoggedData(file, property, objectAttrs).data());
            if (params.value(CLEARLOG, "") != "false"){
                file->remove();
                file->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
            }
        }
    }
    return true;
}

void TasPropertyLogger::stopLogger(QObject* object, QHash<QString, QString> params)
{
    if(mTargets.contains(object)){
        QString errorMsg;
        if(validateParams(params, errorMsg)){
            QString property = params.value(PROPERTY);
            QHash<QString,QFile*> fileMap = mTargets.value(object);
            if(fileMap.contains(property)){
                QFile* file = fileMap.value(property);
                file->remove();
                delete file;
                fileMap.remove(property);
            }
            if(fileMap.isEmpty()){
                mTargets.remove(object);
            }
        }
    }
}

bool TasPropertyLogger::validateParams(QHash<QString, QString> params, QString& errorMsg)
{
    if(!params.contains(PROPERTY)){
        errorMsg = "A property name must be given!";
        return false;
    }
    return true;
}
