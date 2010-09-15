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
#include <QLibrary>
#include <QPluginLoader>
#include <QDebug>
#include <QDir>
#include <QMutableHashIterator>
#include <QStringList>
#include <QLibraryInfo>

#include "taspluginloader.h"
#include "taslogger.h"


/*!
    \class TasPluginLoader
    \brief Loads tastraverser helper plugins and tasfixture plugins.
        
    Loads tas traversal helper plugins. Helper plugins are loaded based on the QObject or QGraphicsItem
    details. Detection is done using the class name and inheritance details.
    TasFixturePlugins are loaded simply based on the path given as parameter. The most common place to look
    for the plugins is under QT plugins directory.

    All plugin load functions may return null if no plugin can be loaded.

    The plugin loader class will store will unload all of the pluginloaders when deleted.
    Note that the unload can fail if other loaders are using the same plugin.
*/

static QString FIXTURE_DIR = "tasfixtures";

/*!
  Constuctor
*/
TasPluginLoader::TasPluginLoader()
{
}

/*!
  Destructor
*/
TasPluginLoader::~TasPluginLoader()
{
    mFixturePlugins.clear();
}


/*!
  Tries to load a fixture plugin based on the path given. When initialized the all plugins
  from QT_PLUGINS/tasfixtures are loaded and if the given path name matches then it is returned.
  If the plugin is not found from the loaded plugins the it a load is attempted.
  If no plugins is loaded null is returned.
*/
TasFixturePluginInterface* TasPluginLoader::loadFixtureInterface(const QString& pluginPath)
{
    TasFixturePluginInterface* fixture = 0;
    QStringList plainName = pluginPath.split(".");
    if(mFixturePlugins.contains(plainName[0])){
        fixture = mFixturePlugins.value(pluginPath);
    }
    else{
        fixture = tryToLoadFixture(pluginPath, plainName[0]);
    }
    return fixture;
}

/*!
  Loads all plugins from QT_PLUGINS/tasfixtures directory into to a local cache.
 */
void TasPluginLoader::initializeFixturePlugins()
{
    QStringList plugins = listPlugins(FIXTURE_DIR);
    QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath) + "/"+FIXTURE_DIR;
    for (int i = 0; i < plugins.count(); ++i) {
        QString fileName = plugins.at(i);
        QString filePath = QDir::cleanPath(path + QLatin1Char('/') + fileName);
        if(QLibrary::isLibrary(filePath)){
            QStringList plainName = fileName.split(".");

#if defined(Q_OS_LINUX) || defined(Q_WS_MAC) 
            tryToLoadFixture(filePath, plainName[0].mid(3));
#else
            tryToLoadFixture(filePath, plainName[0]);
#endif
        }
    }
}

QStringList TasPluginLoader::listPlugins(QString pluginDir)
{
#ifdef Q_OS_SYMBIAN
    //TEMPORARY workaround:
    //there is a defect in s60 qt port so that it does not search for plugins 
    //from all possible drives, so check for existence before loading the plugin
    //issue has been reported to port team and they will fix it at some point
    QString flashDrive = "C:";
    QString romDrive = "Z:";   
    QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath) + "/"+pluginDir;
    //add drive letter to plugin path and then check c and z for the plugin
    if(!path.startsWith(flashDrive,Qt::CaseInsensitive) && !path.startsWith(romDrive,Qt::CaseInsensitive)){
        path = flashDrive + path;
    }
    QStringList plugins = QDir(path).entryList(QDir::Files);

    if(path.startsWith(flashDrive,Qt::CaseInsensitive)){
        path.replace(flashDrive, romDrive, Qt::CaseInsensitive);
    }        
    else{
        path.replace(romDrive, flashDrive, Qt::CaseInsensitive);
    }
    plugins.append(QDir(path).entryList(QDir::Files));

#else   
    QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath) + "/"+pluginDir;
    QStringList plugins = QDir(path).entryList(QDir::Files);
#endif
    return plugins;
}


/*!
  Try to load a plugin from the given path. Returns null if no plugin loaded.
 */
TasFixturePluginInterface* TasPluginLoader::tryToLoadFixture(QString filePath, QString id)
{
    TasFixturePluginInterface* fixture = 0; 
    QObject *plugin = loadPlugin(filePath);
    if(plugin){
        fixture = qobject_cast<TasFixturePluginInterface *>(plugin);        
        if (fixture && !mFixturePlugins.contains(id)){
            mFixturePlugins.insert(id, fixture);            
        }
    }    
    return fixture;
}

/*!
  Perform plugin load.
 */
QObject* TasPluginLoader::loadPlugin(QString pluginLoadPath)
{     
#ifdef Q_OS_SYMBIAN
    //TEMPORARY workaround:
    //there is a defect in s60 qt port so that it does not search for plugins 
    //from all possible drives, so check for existence before loading the plugin
    //issue has been reported to port team and they will fix it at some point
    QString flashDrive = "C:";
    QString romDrive = "Z:";   

    //add drive letter to plugin path and then check c and z for the plugin
    if(!pluginLoadPath.startsWith(flashDrive,Qt::CaseInsensitive) && !pluginLoadPath.startsWith(romDrive,Qt::CaseInsensitive)){
        pluginLoadPath = flashDrive + pluginLoadPath; 
    }

    QPluginLoader loader(pluginLoadPath);
        
    QObject *plugin = loader.instance();

    if(!plugin){
        if(pluginLoadPath.startsWith(flashDrive,Qt::CaseInsensitive)){
            pluginLoadPath.replace(flashDrive, romDrive, Qt::CaseInsensitive);
        }        
        else{
            pluginLoadPath.replace(romDrive, flashDrive, Qt::CaseInsensitive);
        }
        loader.setFileName(pluginLoadPath);
        plugin = loader.instance(); 
    }
    //if the file is in neither then let failure occur similarly as with other platforms

#else

    QPluginLoader loader(pluginLoadPath);
    QObject *plugin = loader.instance();
#endif
    if(!plugin){
        TasLogger::logger()->error("Plugin load failed. Reason: " + loader.errorString()); 
    }

    return plugin;
}

