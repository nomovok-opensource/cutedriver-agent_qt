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


#ifndef DIALOGFIXTUREPLUGIN_H
#define DIALOGFIXTUREPLUGIN_H

#include <QObject>
#include <QHash>
#include <QStringList>

class QRegExp;

#include <tasqtfixtureplugininterface.h>
#include <tasqtdatamodel.h>
#include <tasconstants.h>
#include <tasuitraverser.h>

class DialogFixture : public QObject, public TasFixturePluginInterface
{
    Q_OBJECT
    Q_INTERFACES(TasFixturePluginInterface)

public:
            DialogFixture(QObject* parent=0);
    ~DialogFixture();
    bool execute(void* objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut);

    // common methods in dialogfixture.cpp

private:
    // non-platform specific private declarations, definitions are in dialogfixture.cpp

    //
    void mangleWhiteSpace(QString &str);


private:
    // platform specific methods, definitions are in dialogfixture_*.cpp files

    // dumpDialog returns multi-line string where each line has three elements: handle class name
    // handle are class are separated are separated and followed by one whitespace
    // everything after the second whitespace is item name, which may be empty
    // every line ends with '\n'
    QString dumpDialog(const QString &dialogName);

    // functions below return:
    //    <0 for system error, exact value is undefined
    //     0 for not found error
    //     1 for success
    //    >1 for multiple matches error, value is match count

    int tap(const QRegExp &childName, const QRegExp &childClass, const QString &dialogName);
    int sendText(const QString &text, const QRegExp &childName, const QRegExp &childClass, const QString &dialogName);

};


#endif

