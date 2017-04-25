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


#include <QtGui>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QApplication>
#include <QDesktopWidget>

#include "serverwindow.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QApplication::setOrganizationName("Nokia");
    QApplication::setOrganizationDomain("nokia.com");
    QApplication::setApplicationName("QtTasserverUi");

    bool isFullscreen = false;
    QStringList arguments = app.arguments();
    for (int i = 0; i < arguments.count(); ++i) {
        QString parameter = arguments.at(i);
        if (parameter == "-fullscreen") {
            isFullscreen = true;
        } else if (parameter == "-help") {
            qDebug() << "cuTeDriver Qttas_ui application.";
            qDebug() << "-fullscreen   - show QML fullscreen";
            qDebug() << "-help  - This message";
            exit(0);
        }
    }

    ServerWindow serverWindow;

    if (isFullscreen) {
        serverWindow.resize(QApplication::desktop()->size());
        serverWindow.showFullScreen();
    } else {
        serverWindow.setFixedSize(350,600);
        serverWindow.show();
    }

    return app.exec();
}



