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



#include <QGraphicsWidget>
#include <QMouseEvent>
#include <QLocale>

#if defined(TAS_MAEMO) && defined(HAVE_QAPP)
#include <MApplication>
#include <MLocale>
#include <MCalendar>
#endif

#include "localefixture.h"
#include "testabilityutils.h"
#include "taslogger.h"

static const QString DAYNUMBER = "daynumber";
static const QString MONTHNUMBER = "monthnumber";
static const QString TIMESTRING = "timestring";
static const QString DATESTRING = "datestring";
static const QString CURRENCYAMOUNT = "currencyamount";
static const QString CURRENCYTYPE = "currencytype";
static const QString NUMBERVALUE = "numbervalue";


Q_PLUGIN_METADATA(LocaleFixturePlugin)

/*!
  \class LocaleFixturePlugin
  \brief give access to QLocale/MLocale values from scripts
*/

LocaleFixturePlugin::LocaleFixturePlugin(QObject* parent) :QObject(parent) {}
LocaleFixturePlugin::~LocaleFixturePlugin() {}

bool LocaleFixturePlugin::execute(
    void* objectInstance,
    QString actionName,
    QHash<QString, QString> parameters,
    QString& stdOut)
{
    TasLogger::logger()->debug("> LocaleFixturePlugin::execute translation");
    bool result = true;

#if defined(TAS_MAEMO) && defined(HAVE_QAPP)
    QLocale defaultQLocale;
    MApplication* app = MApplication::instance();
    MLocale defaultMLocale;
    MCalendar *myCalendar = new MCalendar();

    if (actionName == "getDayName") {
        int day = parameters[DAYNUMBER].toInt();
        if (day < 1 || day > 7){
            stdOut.append("Error: Weekday number out of range");
            return false;
        }
        if (app){
            stdOut.append(defaultMLocale.weekdayName(*myCalendar, day));
        }
        else
        {
            stdOut.append(defaultQLocale.dayName(day));
        }
    }
    else if (actionName == "getMonthName") {
        int month = parameters[MONTHNUMBER].toInt();
        if (month < 1 || month > 12){
            stdOut.append("Error: Month number out of range");
            result = false;
        }
        if (app){
            stdOut.append(defaultMLocale.monthName(*myCalendar, month));
        }
        else
        {
            stdOut.append(defaultQLocale.monthName(month));
        }
    }
    else if (actionName == "getNumber") {
        double value = parameters[NUMBERVALUE].toDouble();
        if (app){
            stdOut.append(defaultMLocale.formatNumber(value));
        }
        else
        {
            // type g & max 6 decimals
            stdOut.append(defaultQLocale.toString(value, 'g', 6));
        }
    }
    else if (actionName == "getDate") {
        QDateTime tempdate = QDateTime::fromString(parameters[DATESTRING], "d'.'MM'.'yyyy");
        if(tempdate.isValid())
        {

            if (app){
                QString formattedDateTime;
                myCalendar->setDateTime(tempdate);
                formattedDateTime = defaultMLocale.formatDateTime(*myCalendar, MLocale::DateMedium, MLocale::TimeNone);
                stdOut.append(formattedDateTime);
            }
            else
            {
                stdOut.append(defaultQLocale.toString(tempdate, defaultQLocale.dateFormat()));
            }
        }
        else
        {
            stdOut.append("Datevalue was not valid, should be d.mm.yyyy");
            result = false;
        }
    }

    else if (actionName == "getTime") {
        QString timevalue = parameters[TIMESTRING];
        QDateTime temptime = QDateTime::fromString(timevalue, "h':'m':'ss");
        if(temptime.isValid()){

            if (app){
                QString formattedDateTime;
                //QDateTime current = QDateTime::currentDateTime();
                myCalendar->setDateTime(temptime);
                formattedDateTime = defaultMLocale.formatDateTime(*myCalendar, MLocale::DateNone, MLocale::TimeMedium);
                stdOut.append(formattedDateTime);
            }
            else
            {
                stdOut.append(defaultQLocale.toString(temptime, defaultQLocale.timeFormat()));
            }
        }
        else
        {
            stdOut.append("Timevalue was not valid, should be h:m:ss");
            result = false;
        }
    }

    else if (actionName == "getCurrency") {
        if (app){
            double amount = parameters[CURRENCYAMOUNT].toDouble();
            QString currency = parameters[CURRENCYTYPE];
            //If currency is empty string, format currency returns default currency of locale. Undocumented behauviour, but works.
            stdOut.append(defaultMLocale.formatCurrency(amount, currency));
        }
        else{
            stdOut.append("currency not supported in QLocale until Qt4.8");
        }
    }
    else if (actionName == "getSeparator") {
        stdOut.append(defaultQLocale.groupSeparator());
    }
    else if (actionName == "getDecimalPoint") {
        stdOut.append(defaultQLocale.decimalPoint());
    }
    else {
        stdOut.append("no fixture method found");
        result = false;
    }
    TasLogger::logger()->debug("< LocaleFixturePlugin::execute");
    return result;

#else
    stdOut.append("LocaleFixture implemented only for Meego");
    return false;
#endif


}
