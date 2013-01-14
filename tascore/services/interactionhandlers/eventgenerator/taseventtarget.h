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

#ifndef TASEVENTTARGET_H
#define TASEVENTTARGET_H

class QPoint;
class QWidget;
class QWindow;

#include "uicommandservice.h"

class TasEventTarget
{
public:
    TasEventTarget(QWidget* ptarget = 0, QWindow* ptargetWindow = 0);
    TasEventTarget(TargetData td);

    QWidget* target;
    QWindow* targetWindow;

    QObject* receiver() const;
    QPoint mapFromGlobal(const QPoint& point) const;
    QPoint mapToGlobal(const QPoint& point) const;

    QWidget* parentWidget() const;
    WId winId() const;
};


#endif
