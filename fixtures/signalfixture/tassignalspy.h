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


#ifndef TASSIGNALSPY_H
#define TASSIGNALSPY_H

#include <QApplication>
#include <QObject>
#include <QSignalSpy>
#include <QWidget>
#include <tasqtdatamodel.h>
#include <tasconstants.h>
#include "tastraverseinterface.h"

class TasSignalSpy : public QObject
{
    Q_OBJECT

public:
    TasSignalSpy(QObject * object, const char * signal, TasObjectContainer& objectContainer, bool traverseSender=false);
    ~TasSignalSpy();

    void setTarget(QObject* target);

protected slots:
    void signalHasOccured();

private:
    QString mSignalName;
    QString mSenderClassName;
    QString mSenderId;
    TasObjectContainer& mObjectContainer;
    QSignalSpy* mSignalSpy;
    bool mTraverseSender;
    QHash<QString, TasTraverseInterface*> mTraversers;
    QObject* mTarget;
};

#endif
