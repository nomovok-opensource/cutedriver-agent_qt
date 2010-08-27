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



#include <QApplication>
#include <QtPlugin>
#include <QDebug>
#include <QHash>

#include <taslogger.h>
#include <testabilityutils.h>
#include <tasqtdatamodel.h>

#include "filefixture.h"



Q_EXPORT_PLUGIN2(filefixture, FileFixture)

/*!
  \class FileFixture
  \brief Provides native file access.

  Provides actions for file reading, writing, removing,
  and directory creation, removal and listing.
*/

/*!
  Constructor
*/
FileFixture::FileFixture(QObject* parent)
    :QObject(parent)
{}

/*!
  Destructor
*/
FileFixture::~FileFixture()
{}

/*!
  Implementation for traverse so always true.
*/
bool FileFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    Q_UNUSED(objectInstance);

    TasLogger::logger()->debug("> FileFixture::execute:" + actionName);

    if (actionName == "read_file") {
        readFile(parameters.value(FILE_NAME), stdOut);
    } else if(actionName == "write_file") {
        writeFile(parameters.value(FILE_NAME), parameters.value(FILE_DATA), stdOut);
    } else if(actionName == "delete_file") {
        deleteFile(parameters.value(FILE_NAME), stdOut);
    } else if(actionName == "mk_dir") {
        mkDir(parameters.value(FILE_NAME), stdOut);
    } else if(actionName == "rm_dir") {
        rmDir(parameters.value(FILE_NAME), stdOut);
    } else if(actionName == "list_files") {
        listFiles(parameters.value(FILE_PATH), parameters.value(FILE_NAME), stdOut);
    }

    return true;
}

void FileFixture::readFile(QString fileName, QString& stdOut)
{
    TasLogger::logger()->debug("> FileFixture::readFile:" + fileName);
    QFile file(fileName);
    if(file.exists()) {
//        TasLogger::logger()->debug("> FileFixture::readFile: file exists");

        file.open(QIODevice::ReadOnly);

        if(file.isOpen() && file.isReadable())
        {
            QByteArray array = file.readAll().toBase64();
            stdOut.append(array);
            file.close();
            stdOut.append(FILE_OK);
        }
        else {
            TasLogger::logger()->debug("> FileFixture::writeFile: file could not be open for reading" + fileName);
            stdOut.append(FILE_FAILED);
        }
    }
    else
    {
        stdOut.append(FILE_FAILED);
    }
}

void FileFixture::writeFile(QString fileName,QString fileIn, QString& stdOut)
{
    TasLogger::logger()->debug("> FileFixture::writeFile:" + fileName);
    QFile file(fileName);

    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if(file.isOpen() && file.isWritable())
    {
        QByteArray array = QByteArray::fromBase64(fileIn.toUtf8());
        file.write(array);
        file.close();
        stdOut.append(FILE_OK);
    }
    else {
        TasLogger::logger()->debug("> FileFixture::writeFile: file could not be open for writing" + fileName);
        stdOut.append(FILE_FAILED);
    }
}

void FileFixture::deleteFile(QString fileName, QString& stdOut)
{
    TasLogger::logger()->debug("> FileFixture::deleteFile:" + fileName);
    QFile file(fileName);

    if(file.exists())
    {
        if(file.remove(fileName)){

            stdOut.append(FILE_OK);
        }
        else {
            stdOut.append(FILE_FAILED);
        }
    }
    else
    {
        stdOut.append(FILE_FAILED);
    }
}

void FileFixture::mkDir(QString dirName, QString& stdOut)
{
    TasLogger::logger()->debug("> FileFixture::mkDir:" + dirName);
    QDir dir(dirName);
    if(!dir.exists()) {
        if(!dir.mkpath(dirName)){
            TasLogger::logger()->debug("> FileFixture::mkDir: failed to create dir");
            stdOut.append(FILE_FAILED);
        }
        else {
            stdOut.append(FILE_OK);
        }
    }
    else {
        stdOut.append(FILE_OK);
    }
}

void FileFixture::recursiveRmDir(QDir dir)
{
    TasLogger::logger()->debug("FileFixture::recursiveRmDir: " + dir.absolutePath());
    //iterate all files and subfolder, then delete this dir
    QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for(int n=0;n<list.size();n++){
        QFileInfo info = list.at(n);
        TasLogger::logger()->debug("  > " + info.absoluteFilePath());

        if(info.isDir()){
            QDir sub_dir(info.absoluteFilePath());
            recursiveRmDir(sub_dir);
            dir.rmdir(sub_dir.absolutePath());
        }
        else if(info.isFile())
        {
            QFile file;
            file.remove(info.absoluteFilePath());
        }
    }
}


void FileFixture::rmDir(QString dirName, QString& stdOut)
{
    TasLogger::logger()->debug("FileFixture::rmDir:" + dirName);
    QDir dir(dirName);
    if(dir.exists()) {
        //delete sub dirs and files
        recursiveRmDir(dir);

        //delete target dir
        if(dir.rmdir(dir.absolutePath())){
            TasLogger::logger()->debug("  > dir removed");
            stdOut.append(FILE_OK);
        }
        else {
            TasLogger::logger()->debug("  > failed to remove dir");
            stdOut.append(FILE_FAILED);
        }
    }
    else {
        stdOut.append(FILE_FAILED);
    }
}

void FileFixture::listFiles(QString path, QString fileName, QString& stdOut)
{
    TasLogger::logger()->debug("> FileFixture::listFiles " + path + " " + fileName);

    QDirIterator it(path,
                    QStringList(fileName),
                    QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

    while(it.hasNext()){
        stdOut.append(it.next());
        if(it.hasNext())
            stdOut.append(";");
    }
//    TasLogger::logger()->debug("> FileFixture::listFiles \n" + stdOut);
}
