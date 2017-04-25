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



#ifndef FINDOBJECTSERVICE_H
#define FINDOBJECTSERVICE_H

#include <QByteArray>

#include "tasqtdatamodel.h"
#include "tasuitraverser.h"
#include "tastraverseinterface.h"
#include "tasservicebase.h"

class FindObjectService : public TasServiceBase
{
public:
    FindObjectService();
    ~FindObjectService();

    /*!
      From ServiceInterface
    */
    bool executeService(TasCommandModel& model, TasResponse& response);
    QString serviceName()const { return FIND_OBJECT_SERVICE; }

private:
    bool addObjectDetails(TasObject& parent, TasTargetObject *targetObj, TasCommand* command, QObject* parentObject=0);
    QList<QObject*> searchForObject(TasTargetObject *targetObj);
    QList<QObject*> findMatchingObject(QList<QObject*> objectList, TasTargetObject *targetObj);
    bool isMatch(QObject* candidate, TasTargetObject *targetObj);
    bool propertiesMatch(QHash<QString,QString>, QObject* object);

private:
    TasUiTraverser* mTraverser;
    TasTraverseUtils* mTraverseUtils;
};

#endif
