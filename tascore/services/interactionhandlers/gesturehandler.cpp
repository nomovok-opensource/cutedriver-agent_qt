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
 


#include <testabilityutils.h>

#include <QTimeLine>

#include "taslogger.h"
#include "gesturehandler.h"
#include "tasgesturerunner.h"

/*!
  \class GestureHandler
  \brief GestureHandler generates mouse press and release events.

*/    


GestureHandler::GestureHandler()
{
    mFactory = new TasGestureFactory();    
}

GestureHandler::~GestureHandler()
{    
    delete mFactory;
}

/*!
  Starts a gesture motion based on the given arguments from TasCommand. 
  The gesture is done using a mouse press, move and release operations.
  The path is determined from the given arguments and a QLineF or a list
  of points build from them.

  QTimeLine is used to make the gesture operations. The value from the valuechanged signal
  of timeline is used to determine the position.
 */
bool GestureHandler::executeInteraction(TargetData data)
{
    bool wasConsumed = false;
    TasGesture* gesture = mFactory->makeGesture(data);
    if(gesture){
        wasConsumed = true;
        //will delete it self when done
        TasLogger::logger()->debug("GestureHandler::executeInteraction make gesturerunner");
        new TasGestureRunner(gesture);
    }
    return wasConsumed;
}
