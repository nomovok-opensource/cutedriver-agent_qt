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
 

#include <QDebug>
#include <QDateTime>

#include <taslogger.h>
#include <tascoreutils.h>

#include "tastraverserloader.h"
#include "tassignalspy.h"

// constructor
TasSignalSpy::TasSignalSpy(QObject * object, const char * signal, TasObjectContainer& objectContainer, bool traverseSender)
    :mObjectContainer(objectContainer)
{    
    QObject::connect(object, signal, this, SLOT(signalHasOccured()));
    mSignalSpy = new QSignalSpy(object, signal);
	mSignalName = QString(signal).replace(QString::number(QSIGNAL_CODE), QString(""));
    setTarget(object);
    mTraverseSender = traverseSender;
    if(mTraverseSender){
        TasTraverserLoader loader;
        mTraversers = loader.loadTraversers();
    }
}

// destructor
TasSignalSpy::~TasSignalSpy()
{
    delete mSignalSpy;
    if(!mTraversers.isEmpty()){
        QMutableListIterator<TasTraverseInterface*> i(mTraversers);
        while (i.hasNext()){
            delete i.next();
        }
        mTraversers.clear();
    }
}

void TasSignalSpy::setTarget(QObject* target)
{
    mTarget = target;
    mSenderClassName = QString(mTarget->metaObject()->className());
    mSenderId = TasCoreUtils::objectId(mTarget);
}

/*!
  Slot for capturing signals that the signal spy gets.
 */
void TasSignalSpy::signalHasOccured()
{
    TasLogger::logger()->debug("TasSignalSpy::signalHasOccured signal:" + mSignalName + " sender:" + mSenderClassName);
    // retrieve current time
	QDateTime timeStamp = QDateTime::currentDateTime();

    TasObject& signalData = mObjectContainer.addNewObject(timeStamp.toString(DATE_FORMAT), mSignalName, "QtSignal");

    signalData.addAttribute( "signalName", mSignalName);
    signalData.addAttribute( "senderObjectType", mSenderClassName);
    signalData.addAttribute( "senderObjectId", mSenderId);					
    signalData.addAttribute( "timeStamp", timeStamp.toString(DATE_FORMAT) );
    
    //take arguments if any
    if(!mSignalSpy->isEmpty()){
        QList<QVariant> arguments = mSignalSpy->takeFirst();
        for(int i = 0 ; i < arguments.size(); i++){
            QVariant argument = arguments.at( i );
            TasObject& item_object_arguments = signalData.addNewObject( QString::number(i), "", "QtSignalArgument" );
            item_object_arguments.addAttribute( "type", argument.typeName());
            item_object_arguments.addAttribute( "value", argument.toString());
        } 
    }

    if(mTraverseSender){
        TasObject& targetObj = signalData.addObject();
        for (int i = 0; i < mTraversers.size(); i++) {
            mTraversers.at(i)->traverseObject(&targetObj, mTarget);
        }            
    }
}

