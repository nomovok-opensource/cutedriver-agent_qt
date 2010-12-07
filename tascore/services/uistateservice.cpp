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
 


#include "uistateservice.h"
#include "taslogger.h"
#include "testabilityutils.h"
#include "tastraverserloader.h"

/*!
  \class UiStateService
  \brief UiStateService provides the ui state of the app

*/    

UiStateService::UiStateService()
{
    TasTraverserLoader loader;
    mTraverser = new TasUiTraverser(loader.loadTraversers());
}

UiStateService::~UiStateService()
{
    delete mTraverser;
}

bool UiStateService::executeService(TasCommandModel& model, TasResponse& response)
{    
    if(model.service() == serviceName() ){
        response.setData(makeUiState(model));
        return true;
    }
    else{
        return false;
    }   
}

QByteArray* UiStateService::makeUiState(TasCommandModel& model)
{
    TasCommand* command = 0;
    if (model.targetList().size() > 0) {
        // Pass first command to traverse, if available.
        command = model.targetList().at(0)->commandList().at(0);
    }
    QByteArray* xml = new QByteArray(); 
    TasDataModel* uiModel = mTraverser->getUiState(command);
    uiModel->serializeModel(*xml, 0, model.onlyFragment());
    delete uiModel;
    return xml;
}
