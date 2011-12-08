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
#include <QPixmap>
#include <QDesktopWidget>
#include <QBuffer>

#include "tassocket.h"
#include "tasconstants.h"

#include "utilityapp.h"
 

TasUtilityApp::TasUtilityApp()
{}
TasUtilityApp::~TasUtilityApp()
{}

void TasUtilityApp::sendScreenShot(uint id)
{
    QPixmap screenshot = QPixmap::grabWindow(QApplication::desktop()->winId());
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    screenshot.save(&buffer, "PNG");
    sendData(bytes, id);
}


void TasUtilityApp::sendData(const QByteArray& data, uint id)
{
#if defined(TAS_NOLOCALSOCKET)
    QTcpSocket serverConnection;
#else
    QLocalSocket serverConnection;
#endif
    TasClientSocket socket(&serverConnection);
#if defined(TAS_NOLOCALSOCKET)
    serverConnection.connectToHost(QT_SERVER_NAME, QT_SERVER_PORT);
#else
    serverConnection.connectToServer(LOCAL_SERVER_NAME);
#endif   
    serverConnection.waitForConnected(3000);
    socket.sendResponse(id, data);
}
