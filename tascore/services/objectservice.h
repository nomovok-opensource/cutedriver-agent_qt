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



#ifndef OBJECTSERVICE_H
#define OBJECTSERVICE_H

#include "tasservicebase.h"

class ObjectService : public TasServiceBase
{
public:
    ObjectService();
        ~ObjectService();

        /*!
          From ServiceInterface
        */
        bool executeService(TasCommandModel& model, TasResponse& response);
        QString serviceName()const { return OBJECT_SERVICE; }
private:
        void performObjectService(TasCommandModel& model, TasResponse& response);
        void doCallMethod(TasCommand* command, QObject* target, QString& errorString);
        void doSetAttribute(TasCommand* command, QObject* target, QString& errorString);
};

#endif
