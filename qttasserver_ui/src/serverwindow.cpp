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
#include <QDesktopWidget>
#include <QLabel>
#include <QWidget>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QTextEdit>


#include <version.h>
#include "serverwindow.h"


ServerWindow::ServerWindow(QWidget* parent)
    :QWidget(parent)
{
    setWindowTitle("Qt TasServer Ui");

    monitor = new ServerMonitor();

    statusButton = new QPushButton("Check status");
    stopButton = new QPushButton("Stop");
    startButton = new QPushButton("Start");
    resetButton =  new QPushButton ("Reset server");
#ifdef Q_OS_SYMBIAN
    pluginButton = new QPushButton ("Enable tas");    
    autoStart = new QCheckBox("Autostart"); 
    autoStart->setTristate(false);
    if(monitor->autostartState()){
        autoStart->setCheckState(Qt::Checked);
    }
    connect(autoStart, SIGNAL(toggled(bool)), monitor, SLOT(setAutoStart(bool)));
#endif

    QLabel* stateLabel = new QLabel("Server state:");
    QLabel* versionLabel = new QLabel("Server version:");   
    QLabel* stateValue = new QLabel("Unknown");
    QLabel* versionValue = new QLabel(TAS_VERSION);

    connect(monitor, SIGNAL(serverState(const QString&)), stateValue, SLOT(setText(const QString&)));

    connect(monitor, SIGNAL(beginMonitor()), this, SLOT(disableButtons()));
    connect(monitor, SIGNAL(stopMonitor()), this, SLOT(enableButtons()));


    QTextEdit* editField = new QTextEdit();
    editField->setReadOnly(true);
    connect(monitor, SIGNAL(serverDebug(const QString&)), editField, SLOT(append(const QString&)));

    connect(startButton, SIGNAL(clicked()), editField, SLOT(clear()));
    connect(stopButton, SIGNAL(clicked()), editField, SLOT(clear()));
    connect(resetButton, SIGNAL(clicked()), editField, SLOT(clear()));
    connect(statusButton, SIGNAL(clicked()), editField, SLOT(clear()));

#ifdef Q_OS_SYMBIAN
    connect(pluginButton, SIGNAL(clicked()), editField, SLOT(clear()));
    connect(pluginButton, SIGNAL(clicked()), monitor, SLOT(enablePluginLoad()));
#endif

    connect(statusButton, SIGNAL(clicked()), monitor, SLOT(serverState()));    
    connect(stopButton, SIGNAL(clicked()), monitor, SLOT(stopServer()));
    connect(startButton, SIGNAL(clicked()), monitor, SLOT(startServer()));
    connect(resetButton, SIGNAL(clicked()), monitor, SLOT(restartServer()));    

    QPushButton* quitButton = new QPushButton("Quit");
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(stateLabel, 0, 0, 1, 1);
    mainLayout->addWidget(stateValue, 0, 1, 1, 1);
    mainLayout->addWidget(versionLabel, 1, 0);
    mainLayout->addWidget(versionValue, 1, 1);
    mainLayout->addWidget(editField, 2,0, 1, 2);
#ifdef Q_OS_SYMBIAN
    mainLayout->addWidget(statusButton, 3, 0);
    mainLayout->addWidget(pluginButton, 3, 1);
#else
    mainLayout->addWidget(statusButton, 3, 0, 1, 2);
#endif
    mainLayout->addWidget(stopButton, 4, 0);
    mainLayout->addWidget(startButton, 4, 1);
    mainLayout->addWidget(resetButton, 5, 0);
    mainLayout->addWidget(quitButton, 5, 1);
    mainLayout->addWidget(autoStart, 6, 0, 1, 2);
    setLayout(mainLayout);     

    QRect rect = qApp->desktop()->screenGeometry();    
    if(rect.width() > 864)
        setFixedSize(350,600);
    else{
        showFullScreen();    
    }

}

void ServerWindow::disableButtons()
{
    statusButton->setDisabled(true);
    stopButton->setDisabled(true);
    startButton->setDisabled(true);
    resetButton->setDisabled(true);
}
void ServerWindow::enableButtons()
{
    statusButton->setDisabled(false);
    stopButton->setDisabled(false);
    startButton->setDisabled(false);
    resetButton->setDisabled(false);
}

ServerWindow::~ServerWindow()
{}

