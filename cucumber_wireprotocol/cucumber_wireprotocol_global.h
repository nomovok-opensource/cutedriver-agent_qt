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

#ifndef CUCUMBER_WIREPROTOCOL_GLOBAL_H
#define CUCUMBER_WIREPROTOCOL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CUCUMBER_WIREPROTOCOL_LIBRARY)
#  define CUCUMBER_WIREPROTOCOLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CUCUMBER_WIREPROTOCOLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CUCUMBER_WIREPROTOCOL_GLOBAL_H
