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



#ifndef SCREENSHOTSERVICE_H
#define SCREENSHOTSERVICE_H

#include "tasservicebase.h"

class ScreenshotService : public TasServiceBase
{
public:
    ScreenshotService();
    ~ScreenshotService();

    /*!
      From ServiceInterface
    */
    bool executeService(TasCommandModel& model, TasResponse& response);
    QString serviceName()const { return SCREEN_SHOT; }

private:
    void getScreenshot(TasCommandModel& model, TasResponse& response);
    bool isFormatSupported(QString format);
};

#endif
