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
#include <QObjectCleanupHandler>

#include <taslogger.h>
#include <testabilityutils.h>
#include <tasqtdatamodel.h>

#include "contactfixture.h"

static const QString ACTION_STORE_LIST = "list_contact_stores";
static const QString ACTION_ADD_CONTACT = "add_contact";
static const QString ACTION_REMOVE_ALL_CONTACT = "remove_all_contact";

static const QString STORE_NAME = "store_name";
static const QString CONTACT_FIRSTNAME = "first_name";
static const QString CONTACT_LASTNAME = "last_name";
static const QString CONTACT_MOBILEHOMENUMBER = "mobile_home_number";
static const QString CONTACT_MOBILEWORKNUMBER = "mobile_work_number";

static const QString CONTACT_LANDLINEHOMENUMBER = "landline_home_number";
static const QString CONTACT_LANDLINEWORKNUMBER = "landline_work_number";


Q_PLUGIN_METADATA(ContactFixture)

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
    bool executed = true;
    TasLogger::logger()->debug("> ContactFixture::execute:" + actionName);

    if (actionName.toLower() == ACTION_STORE_LIST) {
        // Find contact stores
        QStringList availableManagers = QContactManager::availableManagers();
        availableManagers.removeAll("invalid");
        QStringListIterator it(availableManagers);

        while(it.hasNext()){
            stdOut.append(it.next());
            if(it.hasNext())
                stdOut.append(";");
        }
    } else{

        QContactManager * manager = connectContactManager(parameters, stdOut);      
        if ( manager == NULL) {
            executed = false;
        }
        else {
            if (actionName.toLower() == ACTION_ADD_CONTACT) {
                executed = addContacts(*manager, parameters, stdOut);
            }
            else if (actionName.toLower() == ACTION_REMOVE_ALL_CONTACT) {
                executed = removeContacts(*manager, parameters, stdOut);
            }
            else {
                stdOut.append("Invalid contact fixture command: "+ actionName);
                executed = false;
            }
            delete manager;
        }        
    }
    return executed;
}

bool ContactFixture::addContacts(QContactManager& manager, QHash<QString, QString> parameters, QString & stdOut)
{
    TasLogger::logger()->debug("> ContactFixture::addContacts");
    
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

    QHashIterator<QString, QString> paramIterator(parameters);
    QString paramKey;
    while(paramIterator.hasNext())
    {
        paramIterator.next();
        paramKey = paramIterator.key();

        // Manage numbers
        if (paramKey.startsWith("number", Qt::CaseInsensitive))
        {
            QStringList numberDetails = paramIterator.value().split(";");
            QContactPhoneNumber phn;
            switch(numberDetails.count())
            {
            case 3: // Context {Home/Work}
                if (numberDetails.at(2)!="")
                {
                    phn.setContexts(numberDetails[2].toLatin1());
                }
            case 2: // Subtype
                if (numberDetails.at(1)!="")
                {
                    phn.setSubTypes(numberDetails[1].toLatin1());
                }
            case 1:
                phn.setNumber(numberDetails[0]);
                break;
            default:
                stdOut.append("Invalid parameters for number, should be \"number;type;context\"");
                return false;
            }
            curr.saveDetail(&phn);
        }
        // Manage emails
        if (paramKey.startsWith("email", Qt::CaseInsensitive))
        {
            QStringList emailDetails = paramIterator.value().split(";");
            QContactEmailAddress email;
            switch(emailDetails.count())
            {
            case 2:
                if (emailDetails.at(1)!="")
                {
                    email.setContexts(emailDetails[1].toLatin1());
                }
            case 1:
                email.setEmailAddress(emailDetails[0]);
                break;
            default:
                stdOut.append("Invalid parameters for email, should be \"email;context\"");
                return false;
            }
            curr.saveDetail(&email);
        }
        // Manage address
        if (paramKey.startsWith("address", Qt::CaseInsensitive))
        {
            QStringList addressDetails = paramIterator.value().split(";");
            QContactAddress address;
            switch(addressDetails.count())
            {
            case 7:
                if (addressDetails.at(6)!="")
                {
                    address.setContexts(addressDetails[6].toLatin1());
                }
            case 6:
                if (addressDetails.at(5)!="")
                {
                    address.setPostOfficeBox(addressDetails[5].toLatin1());
                }
            case 5:
                if (addressDetails.at(4)!="")
                {
                    address.setCountry(addressDetails[4].toLatin1());
                }
            case 4:
                if (addressDetails.at(3)!="")
                {
                    address.setRegion(addressDetails[3].toLatin1());
                }
            case 3:
                if (addressDetails.at(2)!="")
                {
                    address.setLocality(addressDetails[2].toLatin1());
                }
            case 2:
                if (addressDetails.at(1)!="")
                {
                    address.setPostcode(addressDetails[1].toLatin1());
                }
            case 1:
                if (addressDetails.at(0)!="")
                {
                    address.setStreet(addressDetails[0]);
                }
                break;
            default:
                stdOut.append("Invalid parameters for address, should be \"street;postcode;locality;region;country;postofficebox;context\"");
                return false;
            }
            curr.saveDetail(&address);
        }
    }

    // Append contact to the store
    curr = manager.compatibleContact(curr);
    bool success = manager.saveContact(&curr);
    if (!success)
    {
        stdOut.append(QString("Failed to save contact!\n(error code %1)").arg(manager.error()));
    }
    return true;
}

bool ContactFixture::removeContacts(QContactManager& manager, QHash<QString, QString> parameters, QString & stdOut)
{
    QList<QContact> contactList = manager.contacts();
    QList<QContact>::iterator i;
    for (i = contactList.begin(); i != contactList.end(); ++i) {
        manager.removeContact((*i).localId());
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

void ContactFixture::addContact(QHash<QString, QString> /*parameters*/, QString& /*stdOut*/)
{
    TasLogger::logger()->debug("> ContactFixture::addContact ");
}

void ContactFixture::removeContact(QHash<QString, QString> /*parameters*/, QString& /*stdOut*/)
{
    TasLogger::logger()->debug("> ContactFixture::removeContact ");
}
