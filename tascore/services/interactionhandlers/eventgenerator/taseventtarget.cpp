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


#include "taseventtarget.h"

#include <QWindow>
#include <QPoint>
#include <QWidget>

#include "tasgesture.h"

TasEventTarget::TasEventTarget(QWidget* ptarget, QWindow* ptargetWindow) :
    target(ptarget),
    targetWindow(ptargetWindow)
{
}

TasEventTarget::TasEventTarget(TasGesture *tg) :
    target(tg->getTarget()),
    targetWindow(tg->getTargetWindow())
{
}

TasEventTarget::TasEventTarget(TargetData td) :
    target(td.target),
    targetWindow(td.targetWindow)
{
}

QObject *TasEventTarget::receiver() const
{
    if (target) {
        return target;
    } else {
        return targetWindow;
    }
}

QPoint TasEventTarget::mapFromGlobal(const QPoint& point) const
{
    if (target) {
        return target->mapFromGlobal(point);
    } else {
        return targetWindow->mapFromGlobal(point);
    }
}

QPoint TasEventTarget::mapToGlobal(const QPoint &point) const
{
    if (target) {
        return target->mapToGlobal(point);
    } else {
        return targetWindow->mapToGlobal(point);
    }
}

QWidget *TasEventTarget::parentWidget() const
{
    if (target) {
        return target->parentWidget();
    }
    return 0;
}

WId TasEventTarget::winId() const
{
    if (target) {
        return target->window()->winId();
    } else {
        return targetWindow->winId();
    }
}
