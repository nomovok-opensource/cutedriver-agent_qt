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



#include "keyhandler.h"

#include <QTestEventList>
#include <QKeyEvent>

/*!
  \class KeyHandler
  \brief KeyHandler generates keyboard events.

*/


KeyHandler::KeyHandler()
{}

KeyHandler::~KeyHandler()
{}

bool KeyHandler::executeInteraction(TargetData data)
{
    QWidget* target = data.target;
    TasCommand& command = *data.command;

    QString commandName = command.name();
    bool wasConsumed = false;
    if (commandName == "KeyPress")
    {
        doKeyEvent(target, QEvent::KeyPress, (Qt::Key)command.text().toUInt());
        wasConsumed = true;
    }
    else if (commandName == "KeyRelease")
    {
        doKeyEvent(target, QEvent::KeyRelease, (Qt::Key)command.text().toUInt());
        wasConsumed = true;
    }
    else if (commandName == "KeyClick")
    {
        QPointer<QWidget> ptr(target);
        doKeyEvent(target, QEvent::KeyPress, (Qt::Key)command.text().toUInt());
        //make sure that the target is still valid (e.g. close button for dialog..)
        if(ptr){
            doKeyEvent(target, QEvent::KeyRelease, (Qt::Key)command.text().toUInt());
        }
        wasConsumed = true;
    }
    else if (commandName == "TypeText")
    {
        QTestEventList events;
        QString keys = command.text();
        events.addKeyClicks(keys);
        events.simulate(target);
        wasConsumed = true;
    }
    return wasConsumed;
}


void KeyHandler::doKeyEvent(QWidget* target, QEvent::Type type, Qt::Key keyCode)
{
    QString text;
    if(keyCode != Qt::Key_Shift && keyCode != Qt::Key_Control && keyCode != Qt::Key_Alt &&
       keyCode != Qt::Key_AltGr && keyCode != Qt::Key_Meta && keyCode != Qt::Key_Mode_switch){
        text = QString(QTest::keyToAscii(keyCode));
    }

    if(!text.isEmpty() && text.data()->isLetter() && mModifiers & Qt::ShiftModifier){
        text = text.toUpper();
    }

    QKeyEvent* a = new QKeyEvent(type, keyCode, mModifiers, text);
    QSpontaneKeyEvent::setSpontaneous(a);
    //qApp->notify(target, &a);
    if (!target)
        qApp->postEvent(qApp->focusObject(), a);
    else
        qApp->postEvent(target, a);

    if(type == QEvent::KeyPress){
        enableModifiers(keyCode);
    }

    if(type == QEvent::KeyRelease){
        disableModifiers(keyCode);
    }
}

void KeyHandler::enableModifiers(Qt::Key keyCode)
{
    switch(keyCode){
        case Qt::Key_Shift:
            mModifiers = mModifiers | Qt::ShiftModifier;
            break;
        case Qt::Key_Control:
            mModifiers = mModifiers | Qt::ControlModifier;
            break;
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
            mModifiers = mModifiers | Qt::AltModifier;
            break;
        case Qt::Key_Meta:
            mModifiers = mModifiers | Qt::MetaModifier;
            break;
        case Qt::Key_Mode_switch:
            mModifiers = mModifiers | Qt::GroupSwitchModifier;
            break;
        default:
            break;
    }
}

void KeyHandler::disableModifiers(Qt::Key keyCode)
{
    switch(keyCode){
        case Qt::Key_Shift:
            mModifiers = mModifiers ^ Qt::ShiftModifier;
            break;
        case Qt::Key_Control:
            mModifiers = mModifiers ^ Qt::ControlModifier;
            break;
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
            mModifiers = mModifiers ^ Qt::AltModifier;
            break;
        case Qt::Key_Meta:
            mModifiers = mModifiers ^ Qt::MetaModifier;
            break;
        case Qt::Key_Mode_switch:
            mModifiers = mModifiers ^ Qt::GroupSwitchModifier;
            break;
        default:
            break;
    }
}
