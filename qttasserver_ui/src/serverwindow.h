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
 

#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>

#include "servermonitor.h"

class ServerWindow : public QWidget
{
    Q_OBJECT

public:	  
   ServerWindow(QWidget* parent=0);
   ~ServerWindow();

private slots:
   void disableButtons();
   void enableButtons();

private:
   QPushButton* statusButton;
   QPushButton* stopButton;
   QPushButton* startButton;
   QPushButton* resetButton;
   QPushButton* pluginButton;
   QCheckBox* autoStart;
   ServerMonitor* monitor;
};

#endif
