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



#ifndef KEYHANDLER_H
#define KEYHANDLER_H

#include <QApplication>
#include <QPoint>
#include <QEvent>
#include <QCursor>
#include <QWidget>

#include "uicommandservice.h"

class KeyHandler : public InteractionHandler
{
public:
    KeyHandler();
    ~KeyHandler();

    virtual QString handlerName() { return QString("KeyHandler"); }
    bool executeInteraction(TargetData data);

protected:
    void doKeyEvent(QWidget* target, QEvent::Type type, Qt::Key keyCode);
    void enableModifiers(Qt::Key keyCode);
    void disableModifiers(Qt::Key keyCode);

private:
        Qt::KeyboardModifiers mModifiers;

};

#endif
