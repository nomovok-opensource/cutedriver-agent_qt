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

#include "launchfixture.h"

#include <taslogger.h>


#include <e32base.h>
#include <apgcli.h>

Q_PLUGIN_METADATA(LaunchFixture)

/*!
\class LaunchFixture
\brief launhes application on device based on uid

*/

/*!
Constructor
*/
LaunchFixture::LaunchFixture(QObject* parent)
    :QObject(parent)
{}

/*!
  Destructor
*/
LaunchFixture::~LaunchFixture()
{}

/*!
  return always true, and handle the error in ruby.
  error return is "failed with uid:0x12345678" where 0x12345678 is replaced with given uid
*/
bool LaunchFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    bool result = true;

    //TasLogger::logger()->debug("> LaunchFixture::execute:" + actionName);

    if(actionName == "launch_with_uid"){
        bool ok;
        QString temp = parameters.value("UID");
        //TasLogger::logger()->debug("> LaunchFixture::UID: '" + temp + "'");
        TUint uid_i = temp.toUInt(&ok, 0);
        //TasLogger::logger()->debug("  ok: " + QString::number(ok) + " uid: " + QString::number(uid_i));
        TUid uid = TUid::Uid(uid_i);
        //T R A P D starts
        TRAPD(trapErr,
            RApaLsSession session;
            TInt err = session.Connect();
            User::LeaveIfError(err);
            //TasLogger::logger()->debug("  session " );

            CleanupClosePushL(session);
            TApaAppInfo info;
            err = session.GetAppInfo(info,uid);
            User::LeaveIfError(err);
            //TasLogger::logger()->debug("  appinfo " );

            CApaCommandLine* cmdLine = CApaCommandLine::NewLC();
            cmdLine->SetExecutableNameL(info.iFullName);
            cmdLine->SetCommandL(EApaCommandRun);
            User::LeaveIfError(session.StartApp(*cmdLine));
            //TasLogger::logger()->debug("  command line " );
            stdOut.append(QString((QChar*) info.iFullName.Ptr(), info.iFullName.Length()));
            CleanupStack::PopAndDestroy(2);
        );
        //T R A P D ends
        if(trapErr != KErrNone){
            stdOut.append("failed with uid:" +parameters.value("UID") );
        }
    }
    return result;
}

