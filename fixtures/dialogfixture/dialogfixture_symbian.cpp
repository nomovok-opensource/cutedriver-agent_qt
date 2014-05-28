
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

#include "dialogfixture.h"


// it's assumed that TCHAR is wchar_t. If it isn't, then compilation will hopefully fail.


QString DialogFixture::dumpDialog(const QString &dialogName)
{
    return QString(); // not implemented
}


int DialogFixture::tap(const QRegExp &childName, const QRegExp &childClass, const QString &dialogName)
{
    return -2; // not implemented
}


int DialogFixture::sendText(const QString &text, const QRegExp &childName, const QRegExp &childClass, const QString &dialogName)
{
    return -2; // not implemented
}
