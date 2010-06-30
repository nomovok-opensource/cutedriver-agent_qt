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
#include <QLibraryInfo>
#include <QDir>

#include "tastraverserloader.h"

static QString TRAVERSE_DIR = "traversers";

/*!
  Returns a list of traversers. Ownership is transfered.
 */
QList<TasTraverseInterface*> TasTraverserLoader::loadTraversers()
{
    QList<TasTraverseInterface*> traversers;
    QStringList plugins = listPlugins(TRAVERSE_DIR);
    QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath) + "/" + TRAVERSE_DIR;
    for (int i = 0; i < plugins.count(); i++) {
        QString fileName = plugins.at(i);
        QString filePath = QDir::cleanPath(path + QLatin1Char('/') + fileName);
        if(QLibrary::isLibrary(filePath)){
            QObject *plugin = loadPlugin(filePath);            
            if(plugin){
                TasTraverseInterface* traverser = qobject_cast<TasTraverseInterface*>(plugin);        
                if (traverser){
#if defined(Q_OS_LINUX) || defined(Q_WS_MAC)                                  
                    traverser->setPluginName(fileName.left(fileName.indexOf(".")).mid(3));
#else
                    traverser->setPluginName(fileName.left(fileName.indexOf(".")));
#endif
                    traverser->resetFilter();
                    traversers.append(traverser);
                }
            }    
        }
    }
    return traversers;
}
