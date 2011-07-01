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



#ifndef STARTAPPSERVICE_H
#define STARTAPPSERVICE_H

#include <QTimer>

#include <tasconstants.h>

#include "tasservercommand.h"

class StartAppService : public TasServerCommand
{
public:
    StartAppService();
    ~StartAppService();

    /*!
          From ServiceInterface
        */
    bool executeService(TasCommandModel& model, TasResponse& response);
    QString serviceName() const { return START_APPLICATION; }

    // static utility methods
    static void launchDetached(const QString& applicationPath, const QStringList& arguments,
                               const QStringList& environmentVars, const QString &workingDirectory,
                               QString &responseData, QString responseErrorMessage);
    static QString searchForApp(const QString& appName, const QString& rootPath);

    static QHash<QString, QString> parseEnvironmentVariables(const QString& env);

private:
    void startApplication(TasCommand& command, TasResponse& response);
    void setRuntimeParams(TasCommand& command);

};


#endif
