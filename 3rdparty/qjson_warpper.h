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

#ifndef QJSON_WARPPER_H
#define QJSON_WARPPER_H

#ifdef USE_INCLUDED_QJSON
// user internal version of qjson
// define defined in qjson.pri, if internal version is needed
#include <qjson/src/parser.h>
#include <qjson/src/serializer.h>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif

#endif // QJSON_WARPPER_H
