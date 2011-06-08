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

#include <QRegExp>
#include <QVariant>
#include <QStringList>
#include <QObject>
#include <QWeakPointer>

class CucumberStepData
{
public:
    QRegExp regExp;
    enum TargetType { Invalid, Undefined, ServerInternal, ServerPlugin, ApplicationPlugin } targetType;
    QWeakPointer<QObject> targetObject;
    const char *targetMethod;
    QVariant source;

    CucumberStepData() : targetType(Invalid), targetMethod(NULL) {}

    CucumberStepData(const CucumberStepData &other) :
        regExp(other.regExp)
      , targetType(other.targetType)
      , targetObject(other.targetObject)
      , targetMethod(other.targetMethod)
      , source(other.source)
    {}

    CucumberStepData(const QRegExp &regExp, TargetType targetType, QObject *targetObject, const char *targetMethod, QVariant source=QVariant()) :
        regExp(regExp)
      , targetType(targetType)
      , targetObject(targetObject)
      , targetMethod(targetMethod)
      , source(source)
    {}

    CucumberStepData(const QRegExp &regExp, TargetType targetType, const QWeakPointer<QObject> targetObject, const char *targetMethod, QVariant source=QVariant()) :
        regExp(regExp)
      , targetType(targetType)
      , targetObject(targetObject)
      , targetMethod(targetMethod)
      , source(source)
    {}

    bool isValid() { return (targetType != Invalid && !targetObject.isNull() && targetMethod != NULL); }
    void invalidate() { targetType = Invalid; targetObject.clear(); targetMethod = NULL; }

    bool hasSource() { return !source.isNull(); }
};

#endif // CUCUMBERSTEPDATA_H
