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


#ifndef FILEFIXTUREPLUGIN_H
#define FILEFIXTUREPLUGIN_H

#include <QObject>
#include <QHash>
#include <QDir>
#include <tasqtfixtureplugininterface.h>

static const QString FILE_NAME = "file_name";
static const QString FILE_PATH = "file_path";
static const QString FILE_DATA = "file_data";

static const QString FILE_OFFSET = "file_offset";
static const QString DATA_LENGHT = "data_lenght";

static const QString FILE_OK = "OK";
static const QString FILE_FAILED = "FAILED";

class FileFixture : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.nokia.testability.FileFixture" FILE "filefixture.json")
    Q_INTERFACES(TasFixturePluginInterface)

public:
     FileFixture(QObject* parent=0);
     ~FileFixture();
     bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);

private:
     void readFile(QString fileName, QString& stdOut);
     void readFilePart(QString fileName, int offset, int lenght, QString& stdOut);

     void writeFile(QString fileName, QString fileIn, QString& stdOut);
     void writeFileAppend(QString fileName, QString fileIn, int offset, int lenght, QString& stdOut);

     void deleteFile(QString dirName, QString& stdOut);

     void mkDir(QString dirName, QString& stdOut);
     void rmDir(QString dirName, QString& stdOut);
     void recursiveRmDir(QDir dir);

     void listFiles(QString dirName, QString fileName, QString& stdOut);

 };

#endif //FILEFIXTUREPLUGIN_H

