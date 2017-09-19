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



#ifndef TASCOREUTILS_H
#define TASCOREUTILS_H

#include <QEvent>
//#include <QTouchEvent>
#include <QString>

#include <QThread>
#include "tasconstants.h"

class TAS_EXPORT Sleeper: public QThread
{
public:
    static void sleep(int ms);

};

class TAS_EXPORT TasCoreUtils
{
public:
    static QString getApplicationName();
    static QString eventType(QEvent* event);
    static QString parseExecutable(QString completePath);
    static void wait(int millis);

    static QString objectId(QObject* object);
    static QString pointerId(void* ptr);
    static bool startServer();
    static bool autostart();
    static QString encodeString(const QString& source);
    static bool isChar(const QChar c);

};

#endif
