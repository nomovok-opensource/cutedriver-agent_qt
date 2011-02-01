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
#include <QTimer>
#include <QRegExp>

#include <taslogger.h>
#include <testabilityutils.h>
#include <tasqtdatamodel.h>
#include <tastraverserloader.h>

#include "dialogfixture.h"


static const QString CLASS_NAME = "className";
static const QString INTERVAL = "interval";


Q_EXPORT_PLUGIN2(dialogfixture, DialogFixture)


/*!
\class DialogFixture
\brief Listens to events to detect a dialog

*/

/*!
Constructor
*/
DialogFixture::DialogFixture(QObject* parent)
    :QObject(parent)
{
    TasLogger::logger()->debug("DialogFixture::DialogFixture");
}

/*!
  Destructor
*/
DialogFixture::~DialogFixture()
{
}


/*!
  Fixture plugin entry point
*/
bool DialogFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & resultString)
{
    Q_UNUSED(objectInstance);
    bool result = true;

    static QString msgPrefix = QString("DialogFixture::execute %1").arg(actionName);
    TasLogger::logger()->debug(msgPrefix);

    // first actions that take no standard parameters

    if(actionName == "fixture_id") {
        resultString = tr("%1: %2 %3").arg(actionName).arg(__DATE__).arg(__TIME__);
    }

    else {
        // then actions that take dialog name

        QString dialogName(parameters.value("dialog_name"));

        if (actionName == "dump_dialog") {
            resultString = dumpDialog(dialogName);
        }

        else {
            // finally actions which take name and class regexp patterns

            QRegExp namePattern(parameters.value("name_pattern"));
            QRegExp classPattern(parameters.value("class_pattern"));
            // note: if hash key is not found, empty string will be returned, which produces valid pattern

            if (!namePattern.isValid() || !classPattern.isValid()) {
                TasLogger::logger()->error((msgPrefix + ": parameter regexp validity error: name_pattern %1, class_pattern %2").
                                           arg(namePattern.isValid()).
                                           arg(classPattern.isValid()));
                result = false;
            }

            else if (actionName == "tap_control") {
                int count = tap(namePattern, classPattern, dialogName);
                if (count != 1) {
                    TasLogger::logger()->error(msgPrefix + ": Dialog child match count:" + QString::number(count));
                    result = false;
                }
            }

            else if (actionName == "send_text") {
                int count = sendText(parameters.value("text"), namePattern, classPattern, dialogName);
                if (count != 1) {
                    TasLogger::logger()->error(msgPrefix + ": Dialog child match count:" + QString::number(count));
                    result = false;
                }
            }

            else {
                TasLogger::logger()->error(msgPrefix + ": Unknown action for this fixture.");
                result = false;
            }
        }
    }

    return result;
}


/*!
  Helper function to produce simplified whitespace-free strings
*/
void DialogFixture::mangleWhiteSpace(QString &str)
{
    static QRegExp wsEx("\\s+");
    str = str.trimmed().replace(wsEx, "_");
}
