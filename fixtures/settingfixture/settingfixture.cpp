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
 

#include <QtPlugin>
#include <QHash>
#include <QHashIterator>
#include <QStringListIterator>

#include "settingfixture.h"


const char* const ORGANIZATION = "settingOrganization";
const char* const APPLICATION = "settingApplication";
const char* const FILE_NAME = "settingFileName";
const char* const FORMAT = "settingFormat";
const char* const SCOPE = "settingScope";
const char* const READ_ACTION = "read";
const char* const READ_ALL_ACTION = "readAll";
const char* const SET_ACTION = "set";
const char* const REMOVE_ACTION = "remove";

Q_EXPORT_PLUGIN2(settingfixture, SettingFixture)

/*!
  \class SettingFixture
  \brief Manipulate QSettings
         
  Set, edit and read QSettings values.
*/

/*!
  Constructor
*/
SettingFixture::SettingFixture(QObject* parent)
    :QObject(parent)
{
}

/*!
  Destructor
*/
SettingFixture::~SettingFixture()
{
}

/*!

*/
bool SettingFixture::execute(void * /*objectInstance*/, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    bool returnValue = true;
    //check the type of settings
    if(parameters.contains(ORGANIZATION)){
        QString organization = parameters.take(ORGANIZATION);        
        QString application;
        if(parameters.contains(APPLICATION)){
            application = parameters.take(APPLICATION);
        }
        QSettings settings(getFormat(parameters), getScope(parameters), organization, application);
        stdOut = editSettings(settings, actionName, parameters);
    }
    else if(parameters.contains(FILE_NAME) && parameters.contains(FORMAT)){        
        QSettings settings(parameters.take(FILE_NAME), getFormat(parameters));
        stdOut = editSettings(settings, actionName, parameters);
    }
    else{
        stdOut = "No organization or file name given for settings.";
        returnValue = false;            
    }

    return returnValue;
}

QString SettingFixture::editSettings(QSettings& settings, const QString& action, QHash<QString, QString> parameters)
{
    QString hashEntity = ":%1 =>'%2'";
    parameters.remove(OBJECT_TYPE);
    QString returnValue;

    if(action == READ_ALL_ACTION){
        QStringListIterator keys(settings.allKeys());
        while (keys.hasNext()){
            QString key = keys.next();
            returnValue.append(hashEntity.arg(key).arg(settings.value(key).toString()));
            if(keys.hasNext()){
                returnValue.append(", ");
            }
        }            
    }
    else{
        QHashIterator<QString, QString> i(parameters);
        while (i.hasNext()) {
            i.next();
            if(action == SET_ACTION){
                settings.setValue(i.key(), QVariant(i.value()));
            }
            else if(action == REMOVE_ACTION){
                settings.remove(i.key());
            }        
            else if(action == READ_ACTION){
                returnValue.append(hashEntity.arg(i.key()).arg(settings.value(i.key()).toString()));
                if(i.hasNext()){
                    returnValue.append(", ");
                }
            }
        }
    }
    if(action == READ_ACTION || action == READ_ALL_ACTION){
        returnValue.prepend("{");
        returnValue.append("}");
    }
    return returnValue;
}

QSettings::Scope SettingFixture::getScope(QHash<QString, QString>& parameters)
{
    QSettings::Scope scope = QSettings::UserScope;
    if(parameters.contains(SCOPE) && parameters.take(SCOPE) == "system"){        
        scope = QSettings::SystemScope;
    }
    return scope;
}

QSettings::Format SettingFixture::getFormat(QHash<QString, QString>& parameters)
{
    QSettings::Format format = QSettings::NativeFormat;
    if(parameters.contains(FORMAT)){
        QString formatString = parameters.take(FORMAT);
        if(formatString == "ini"){
            format = QSettings::IniFormat;
        }
        else if(formatString == "invalid"){
            format = QSettings::InvalidFormat;
        }
    }
    return format;
}

