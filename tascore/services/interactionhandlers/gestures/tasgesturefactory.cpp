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

#include "tasgesturefactory.h"

/*!
    \class TasGestureRecognizer

    \brief Creates a TasGesture from the given data.

    TasGestures are created from the data passed from the testabilitydriver host.
    Recornizers are used to identify and create the correct gestures from the data.
*/
/*!
    \fn TasGesture* TasGestureRecognizer::craete(TargetData data)

    Creates the TasGesture that matches the given data.
*/

/*!
    \class TasGestureFactory

    \brief Finds the correct tasgesturerecognizer and uses it to create a TasGesture

    TasGestures are created from the data passed from the testabilitydriver host.
    The factory stores all of the TasGestureRecognizers which create the actual
    TasGesture.
*/

/*!
  Construct a new TasGestureFactory
 */
TasGestureFactory::TasGestureFactory()
{
    mErrorMessage = "None";
    createRecognizers();
}

/*!
  Destructor for TasGestureFactory.
  Deletes all of the registered recognisers.
 */
TasGestureFactory::~TasGestureFactory()
{
    qDeleteAll(mRecognizers);
    mRecognizers.clear();
}

void TasGestureFactory::addRecognizer(TasGestureRecognizer* recognizer)
{
    mRecognizers.append(recognizer);
}

/*!
  Makes a gesture based on the data given. Will return null if no gesture
  recognizer can be matched to the TargetData.command->commandName. getErrorMessage
  will return the latest error message.
  Note: a gesture recognizer may also return null of the data is not valid.
 */
TasGesture* TasGestureFactory::makeGesture(TargetData data)
{
    QString gestureType = data.command->name();

    foreach(TasGestureRecognizer* recogizer, mRecognizers){
        if(recogizer->isSupportedType(gestureType)){
            TasGesture *gesture = recogizer->create(data);
            // check here that every gesture recognizer does not have to check it
            gesture->setUseTapScreen(data.command->parameter("useTapScreen") == "true");
            return gesture;
        }
    }
    mErrorMessage = "TasGestureFactory::makeGesture: Unknown gesture type " + gestureType + ".";
    return 0;
}

/*!
  Return the latest error message.
 */
QString TasGestureFactory::errorMessage()
{
    return mErrorMessage;
}

void TasGestureFactory::createRecognizers()
{
    addRecognizer(new LineTasGestureRecognizer());
    addRecognizer(new PointsTasGestureRecognizer());
    addRecognizer(new PinchZoomTasGestureRecognizer());
    addRecognizer(new RotationTasGestureRecognizer());
}
