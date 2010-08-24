
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



#include <windows.h>
#include <psapi.h>

#include "dialogfixture.h"

#include <QRegExp>


// it's assumed that TCHAR is wchar_t. If it isn't, then compilation will hopefully fail.

static TCHAR* convertToTCHAR(const QString& qstr)
{
    LPTSTR wString = new TCHAR[qstr.size()+1];
    qstr.toWCharArray(wString);
    wString[qstr.size()] = TCHAR(0);//no null added by default so need to do it
    return wString;
}


static QString convertFromTCHAR(const TCHAR wstr[], int size)
{
    return (size != 0) ? QString::fromWCharArray(wstr, size)
        : QString();
}


static HWND findDialog(const QString &dialogName)
{
    LPTSTR dialog = convertToTCHAR(dialogName);
    HWND hwnd = FindWindow(NULL, dialog);
    delete dialog;
    return hwnd;
}


static const int BUFSIZE=256;


QString DialogFixture::dumpDialog(const QString &dialogName)
{
    QString retval;
    HWND hwnd = findDialog(dialogName);
    if (!hwnd) return retval;

    //retval = dialogName +":\n";

    HWND child = FindWindowEx(hwnd, 0, NULL, NULL);
    TCHAR buf[BUFSIZE];
    int len;

    QString winClass;
    QString winText;
    while (child) {

        len = RealGetWindowClass(child, buf, BUFSIZE);
        winClass = (len != 0) ? convertFromTCHAR(buf, len): "<?>";
        mangleWhiteSpace(winClass);

        len = GetWindowText(child, buf, BUFSIZE);
        winText = convertFromTCHAR(buf, len);

        retval += QString("%1 %2 %3\n").arg((int) child).arg(winClass).arg(winText);
        child = FindWindowEx(hwnd, child, NULL, NULL);
    }
    return retval;
}


static int childFinder(HWND &found, const QRegExp &childName, const QRegExp &childClass, const QString &dialogName)
{
    HWND hwnd = findDialog(dialogName);
    int foundCount = -1;

    if (hwnd) {
        TCHAR buf[BUFSIZE];
        int len;
        foundCount = 0;

        QString winClass;
        QString winText;
        for( HWND child = FindWindowEx(hwnd, 0, NULL, NULL) ; child ; child = FindWindowEx(hwnd, child, NULL, NULL)) {

            if (!childClass.isEmpty()) {
                len = RealGetWindowClass(child, buf, BUFSIZE);
                winClass = convertFromTCHAR(buf, len);
                if (!winClass.contains(childClass)) continue;
            }
            if (!childName.isEmpty()) {
                len = GetWindowText(child, buf, BUFSIZE);
                winText = convertFromTCHAR(buf, len);
                if (!winText.contains(childName)) continue;
            }

            ++foundCount;
            found = child;
        }
    }

    return foundCount;
}


int DialogFixture::tap(const QRegExp &childName, const QRegExp &childClass, const QString &dialogName)
{
    HWND found = NULL;
    int foundCount = childFinder(found, childName, childClass, dialogName);
    if (foundCount == 1) SendMessage(found, BM_CLICK, 0, 0);
    return foundCount;
}


int DialogFixture::sendText(const QString &text, const QRegExp &childName, const QRegExp &childClass, const QString &dialogName)
{
    HWND found = NULL;
    int foundCount = childFinder(found, childName, childClass, dialogName);
    if (foundCount == 1) {
        TCHAR *buf = convertToTCHAR(text);
        SendMessage(found, WM_SETTEXT, 0, (LPARAM)buf);
        delete buf;
    }
    return foundCount;
}


