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
 


#include <QApplication>
#include <QTimer>

#include "closeappservice.h"
#include "tassocket.h"

/*!
  \class CloseAppService
  \brief CloseAppService closes the application

*/    

CloseAppService::CloseAppService()
{
}

CloseAppService::~CloseAppService()
{
}

bool CloseAppService::executeService(TasCommandModel& model, TasResponse& response)
{    
    if(model.service() == serviceName() ){
        //close once the response has been sent
        connect(response.requester(), SIGNAL(messageSent()), this, SLOT(requestQuit()));           
        return true;
    }
    else{
        return false;
    }
}

void CloseAppService::requestQuit()
{
    qApp->setProperty(CLOSE_REQUESTED, QVariant(true));    
    //small delay to allow the server to read the response 
    QTimer::singleShot(100, qApp, SLOT(quit()));
}
