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

#ifndef CUCUMBERUTILS_H
#define CUCUMBERUTILS_H

#include <QString>
#include <QHash>

#include "cucumberstepdata.h"
#include "tasconstants.h"

class TAS_EXPORT CucumberUtils
{
public:
    CucumberUtils();

    static CucumberStepDataMap readSteps(const QString &pluginName);
    static CucumberStepDataMap readAllSteps();

private:
    static void addFileContents(CucumberStepDataMap &map, const QString &filePath);

};

#endif // CUCUMBERUTILS_H
