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
#include <QHash>
#include <QStringList>

#include "utilityapp.h"

int main(int argc, char *argv[])
{   
    QApplication app(argc, argv);            
    QString action;
    uint id = 0;
    QStringList args = app.arguments();
    for(int i = 0; i < args.size(); i++){
        if(args.at(i) == "-i" && args.size() > (i+1) ){
            i++;
            id = args.at(i).toUInt();
        }
        if(args.at(i) == "-a" && args.size() > (i+1) ){
            i++;
            action = args.at(i);
        }
    }
    if(action.isEmpty()){
        return 0;
    }

    if(action == "screenshot"){
        TasUtilityApp util;
        util.sendScreenShot(id);
    }
    return 0;
}
