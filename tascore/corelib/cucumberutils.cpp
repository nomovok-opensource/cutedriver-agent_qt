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

#include "cucumberutils.h"
#include <testabilitysettings.h>
#include <taslogger.h>
#include <tasconstants.h>
#include <tasfixtureplugininterface.h>

#include <QDirIterator>
#include <QFile>
#include <QRegExp>
#include <QFileInfo>

CucumberUtils::CucumberUtils()
{
}


CucumberStepDataMap CucumberUtils::readSteps(const QString &pluginName)
{
    CucumberStepDataMap ret;

    QString path = TestabilitySettings::getBasePath("cucumber");
    QDirIterator it(path, QDirIterator::Subdirectories);
    const QString prefix = pluginName + ".";
    static const QString suffix = ".steps";

    for (QString filePath = it.next()
         ; !filePath.isEmpty()
         ; filePath = it.next() ) {

        TasLogger::logger()->info(QString("CucumberUtils::readSteps processing %1").arg(filePath));

        QString fileName = it.fileName();
        if (fileName.endsWith(suffix) && fileName.startsWith(prefix)  && it.fileInfo().isFile()) {
            TasLogger::logger()->info(QString("CucumberUtils::readSteps trying to read %1").arg(it.fileInfo().canonicalPath()));
            addFileContents(ret, it.fileInfo().canonicalFilePath());
        }
    }

    return ret;
}


CucumberStepDataMap CucumberUtils::readAllSteps()
{
    CucumberStepDataMap ret;

    QString path = TestabilitySettings::getBasePath("cucumber");
    QDirIterator it(path, QDirIterator::Subdirectories);
    static const QString suffix = ".steps";

    for (QString filePath = it.next()
         ; !filePath.isEmpty()
         ; filePath = it.next() ) {

        TasLogger::logger()->info(QString("CucumberUtils::readAllSteps processing %1").arg(filePath));

        if (it.fileName().endsWith(suffix) && it.fileInfo().isFile()) {
            TasLogger::logger()->info(QString("CucumberUtils::readAllSteps trying to read %1").arg(it.fileInfo().canonicalPath()));
            addFileContents(ret, it.fileInfo().canonicalFilePath());
        }
    }

    return ret;
}


static void parseFlags(QHash<QString, QString> &flagMap, QStringList flagList)
{
    foreach(const QString &pair, flagList) {
        int pos = pair.indexOf("=");
        // don't care if flag name is actually empty, and with duplicate flags, overwrite earlier value with new
        if (pos == -1)
            flagMap.insert(pair, QString()); // flag without value
        else
            flagMap.insert(pair.left(pos), pair.mid(pos+1)); // flag with value
    }
}


static void insertStepData(CucumberStepDataMap &map, CucumberStepData &data)
{
    if (!data.regExp.isEmpty()) {
#if 1
        // test against empty, since other code must be able to assume these have value
        if (data.flags.value("action").isEmpty()) {
            data.flags.insert("action", CUCUMBER_STEP_DEFAULTACTION);
        }

        if (data.flags.value("plugin").isEmpty()) {
            int pos = data.stepFileSpec.lastIndexOf('/')+1; // pos >= 0
            int len = qMax(0, data.stepFileSpec.indexOf('.', pos) - pos); // if no '.', len is 0
            data.flags.insert("plugin", data.stepFileSpec.mid(pos, len));
        }
#else
        // responsibility of the caller to get fixture action with code like
        // ...flags.value("action", CUCUMBER_STEP_DEFAULTACTION)
#endif
        map.insert(data.regExp, data);
        TasLogger::logger()->debug(QString("CucumberUtils::insertStepData %1").arg(data.toDebugString()));
    }
    else {
        //TasLogger::logger()->debug(QString("CucumberUtils::addFileContents ignoring incomplete step data %1").arg(data.toDebugString()));
        // else ignore incomplete data
    }
}


void CucumberUtils::addFileContents(CucumberStepDataMap &map, const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {

        const QString fileSpecBase = filePath + ":";

        bool lastEmpty = true;
        CucumberStepData data;
        int lineNum = 0;

        while (!file.atEnd()) {
            QString line = file.readLine();
            ++lineNum;
            QString trimmedLine = line.trimmed();

            TasLogger::logger()->debug(QString("CucumberUtils::addFileContents %1:%2").arg(lineNum).arg(line));

            // skip empty lines
            if (trimmedLine.isEmpty()) {
                lastEmpty = true;
                continue;
            }

            // check for new step
            // format of STEP line: STEP [flag[=value]]...
            if (lastEmpty && trimmedLine.startsWith("STEP")) {
                static const QRegExp whiteSpaceEx("\\s+");
                QStringList splitLine = trimmedLine.split(whiteSpaceEx, QString::SkipEmptyParts);

                if (!splitLine.isEmpty() && splitLine.takeFirst() == "STEP") {

                    // start of next step, store current step data and prepare for new step
                    insertStepData(map, data);
                    data.clear();

                    // get step flags
                    parseFlags(data.flags, splitLine);
                    data.stepFileSpec = fileSpecBase + QString::number(lineNum);

                    lastEmpty = false;
                    continue;
                }
            }

            lastEmpty = false;

            // first non-empty line of step is the regExp, rest are step text
            if (data.regExp.isEmpty()) {
                data.regExp = trimmedLine;
            }
            else {
                data.text.append(line);
            }

        }

        // add last item
        insertStepData(map, data);

        file.close();
    }
    else {
        TasLogger::logger()->warning(QString("CucumberUtils::addFileContents file %1 error: %2").arg(filePath, file.errorString()));
    }

}
