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

#ifndef CUCUMBERSTEPDATA_H
#define CUCUMBERSTEPDATA_H

#include <QString>
#include <QHash>


struct CucumberStepData {
    QString stepFileSpec;
    QString regExp;
    QString text;
    QHash<QString, QString> flags;

    CucumberStepData() {}
    CucumberStepData(const CucumberStepData &other) : stepFileSpec(other.stepFileSpec), regExp(other.regExp), text(other.text), flags(other.flags) {}
    void clear() { stepFileSpec.clear(); regExp.clear(); text.clear(); flags.clear(); }
    QString toDebugString() const { return QString("%1:'%2'='%3' (action=%4, plugin=%5)").arg(stepFileSpec,regExp,text.simplified(), flags.value("action"), flags.value("plugin")); }
};

typedef QHash<QString, CucumberStepData> CucumberStepDataMap;

#endif // CUCUMBERSTEPDATA_H
