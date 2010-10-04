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

#include "contactfixture.h"

static const QString ACTION_STORE_LIST = "list_contact_stores";
static const QString ACTION_ADD_CONTACT = "add_contact";
static const QString ACTION_REMOVE_CONTACT = "remove_contact";

static const QString ACTION_LIST_CONTACT = "list_contact";

static const QString STORE_NAME = "store_name";
static const QString CONTACT_FIRSTNAME = "first_name";
static const QString CONTACT_LASTNAME = "last_name";
static const QString CONTACT_MOBILEHOMENUMBER = "mobile_home_number";
static const QString CONTACT_MOBILEWORKNUMBER = "mobile_work_number";

static const QString CONTACT_LANDLINEHOMENUMBER = "landline_home_number";
static const QString CONTACT_LANDLINEWORKNUMBER = "landline_work_number";


Q_EXPORT_PLUGIN2(contactfixture, ContactFixture)

/*!
\class ContactFixture
\brief Manages contact on device

*/

/*!
Constructor
*/
ContactFixture::ContactFixture(QObject* parent)
    :QObject(parent)
{}

/*!
  Destructor
*/
ContactFixture::~ContactFixture()
{}

/*!
  Implementation for traverse so always true.
*/
bool ContactFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    Q_UNUSED(objectInstance);

    TasLogger::logger()->debug("> ContactFixture::execute:" + actionName);

    // Find contact stores
    QStringList availableManagers = QContactManager::availableManagers();
    availableManagers.removeAll("invalid");

    if (actionName == ACTION_STORE_LIST) {

        QStringListIterator it(availableManagers);

        while(it.hasNext()){
            stdOut.append(it.next());
            if(it.hasNext())
                stdOut.append(";");
        }
    } else if (actionName == ACTION_ADD_CONTACT) {

        QContactManager * manager = connectContactManager(parameters, stdOut);
        if ( manager == NULL) {
            return false;
        }
        else {
            QContact curr;

            // Manage name
            if (parameters.contains(CONTACT_FIRSTNAME))
            {
                QContactName nm = curr.detail(QContactName::DefinitionName);
                nm.setFirstName(parameters.value(CONTACT_FIRSTNAME));
                curr.saveDetail(&nm);
            }
            else
            {
                stdOut.append("Mandatory parameter first_name missing");
                return false;
            }
            if (parameters.contains(CONTACT_LASTNAME))
            {
                QContactName nm = curr.detail(QContactName::DefinitionName);
                nm.setLastName(parameters.value(CONTACT_LASTNAME));
                curr.saveDetail(&nm);
            }
            // Manage numbers
            if (parameters.contains(CONTACT_MOBILEHOMENUMBER))
            {
                QContactPhoneNumber phn = curr.detail(QContactPhoneNumber::SubTypeMobile);
                phn.setNumber(parameters.value(CONTACT_MOBILEHOMENUMBER));
                phn.setContexts(QContactDetail::ContextHome);
                curr.saveDetail(&phn);
            }
            if (parameters.contains(CONTACT_MOBILEWORKNUMBER))
            {
                QContactPhoneNumber phn = curr.detail(QContactPhoneNumber::SubTypeMobile);
                phn.setNumber(parameters.value(CONTACT_MOBILEWORKNUMBER));
                phn.setContexts(QContactDetail::ContextWork);
                curr.saveDetail(&phn);
            }
            if (parameters.contains(CONTACT_LANDLINEHOMENUMBER))
            {
                QContactPhoneNumber phn = curr.detail(QContactPhoneNumber::SubTypeLandline);
                phn.setNumber(parameters.value(CONTACT_LANDLINEHOMENUMBER));
                phn.setContexts(QContactDetail::ContextHome);
                curr.saveDetail(&phn);
            }
            if (parameters.contains(CONTACT_LANDLINEWORKNUMBER))
            {
                QContactPhoneNumber phn = curr.detail(QContactPhoneNumber::SubTypeLandline);
                phn.setNumber(parameters.value(CONTACT_LANDLINEWORKNUMBER));
                phn.setContexts(QContactDetail::ContextWork);
                curr.saveDetail(&phn);
            }

            // Append contact to the store
            curr = manager->compatibleContact(curr);
            bool success = manager->saveContact(&curr);
            if (!success)
            {
                stdOut.append(QString("Failed to save contact!\n(error code %1)").arg(manager->error()));
            }
        }
    } else if (actionName == ACTION_REMOVE_CONTACT) {
        stdOut.append(QString("Remove contact: Not supported yet"));
        return false;
        QContactManager * manager = connectContactManager(parameters, stdOut);
        if ( manager == NULL) {

        }
        else {

        }
    } else if (actionName == ACTION_LIST_CONTACT) {
        stdOut.append(QString("List contacts: Not supported yet"));
        return false;
        QContactManager * manager = connectContactManager(parameters, stdOut);
        if ( manager == NULL) {
            return false;
        }
        else {

        }
    } else {
        stdOut.append("Invalid contact fixture command: "+ actionName);
        return false;
    }

    return true;
}

QContactManager * ContactFixture::connectContactManager(QHash<QString, QString> parameters, QString& stdOut)
{
    QString contactStoreName;
    if (parameters.contains(STORE_NAME))
    {
        contactStoreName = parameters.value(STORE_NAME);
    }
    else
    {
        stdOut.append("Mandatory parameter store_name missing");
        return NULL;
    }

    QMap<QString, QString> params;
    QString managerUri = QContactManager::buildUri(contactStoreName, params);

    QContactManager * manager = QContactManager::fromUri(managerUri);
    if ( manager->error() != QtMobility::QContactManager::NoError) {
        stdOut.append(QString("Failed to open store "+contactStoreName+"\n(error code %1)").arg(manager->error()));
        delete manager;
        manager = NULL;
    }

    return manager;
}

void ContactFixture::addContact(QHash<QString, QString> parameters, QString& stdOut)
{
    TasLogger::logger()->debug("> ContactFixture::addContact ");
}

void ContactFixture::removeContact(QHash<QString, QString> parameters, QString& stdOut)
{
    TasLogger::logger()->debug("> ContactFixture::removeContact ");
}
