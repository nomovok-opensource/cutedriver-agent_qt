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
 

/*
 
	what if scenarios:
		1) singal enabled to e.g button and button object is deleted, does get_signal work anymore due to tasserver cannot retrieve button object?
			
			
 
 
*/

#include <QtPlugin>
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QHash>
#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <tasqtdatamodel.h>
#include <taslogger.h>
#include <tasconstants.h>
#include <tascoreutils.h>

#include "tassignalplugin.h"


static QString CONTAINER_ID = "1";

static QString CHILD_SIGNAL = "child_signal";
static QString CHILD_CLASS  = "child_class";
static QString TRAVERSE_SENDER = "traverse_sender";

Q_EXPORT_PLUGIN2(tassignalplugin, TasSignalPlugin)

/*!
  \class TasFixturePlugin
  \brief TasDuiTraverse traverse DUI components for adding custom details to them
        
  Using standard qt objects it is not always possible to get specific details from the 
  components. This TasHelperInterface implementation will allow us to add details from 
  the wanted dui components that are not accesible through the property or other
  generic way. 
*/

/*!
  Constructor
*/
    TasSignalPlugin::TasSignalPlugin(QObject* parent)
        :QObject(parent)
{
    mOccuredSignals = new TasDataModel();
}

/*!
  Destructor
*/
TasSignalPlugin::~TasSignalPlugin()
{
    QString s;
    clearSignals(s);
    delete mOccuredSignals;
}

/*!
  Execute the given action. Possible actions are: list_signals, get_signal, enable_signal and remove_signals.
*/
bool TasSignalPlugin::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    TasLogger::logger()->debug("TasSignalPlugin::execute action: " + actionName );
    bool result = true;
    // set the stdOut if you wish to pass information back to Testability Driver    
    if (actionName == "list_signals") {                
    	result = listSignals( objectInstance, parameters.value(OBJECT_TYPE), stdOut);    	
    }
    else if (actionName == "get_signal") {            
        result = printSignals(stdOut);
    }    
    else if (actionName == "enable_signal") {
        result = enableSignal(objectInstance, parameters, stdOut);
    }    
    else if (actionName == "remove_signals") {
        result = clearSignals(stdOut);
    }
    else {
        printErrorMsg(parameters, stdOut);
        result =  false;
    }
    return result;
}

/*!
  Print error message.
 */
void TasSignalPlugin::printErrorMsg(QHash<QString, QString> parameters, QString& stdOut)
{            
    QHash<QString, QString>::const_iterator i = parameters.constBegin();    
    stdOut.append("(");
    while (i != parameters.constEnd()) {
        stdOut.append(i.key());
        stdOut.append("=>");
        stdOut.append(i.value());
        stdOut.append(")");
        ++i;        
        }
    stdOut.append("}");
}

/*!
  Serialize the model used by TasSignalSpies to report 
  occured signals.
*/ 

bool TasSignalPlugin::printSignals(QString& stdOut)
{
    SerializeFilter* filter = new SerializeFilter();		    		
    filter->serializeDuplicates(true);		
    //filter is deleted by model
    QByteArray xml;
    mOccuredSignals->serializeModel(xml, filter);    
    stdOut.append(QString::fromUtf8(xml.data()));
    return true;
}

/*!
  Clear signal model.
 */
bool TasSignalPlugin::clearSignals(QString& stdOut)
{
    QHash<QString, TasSignalSpy*>::const_iterator i = spyContainerHash.constBegin();
    while (i != spyContainerHash.constEnd()) {
        TasSignalSpy *tasSpy = i.value();
        delete tasSpy;
        ++i;
    }
    spyContainerHash.clear();
    mOccuredSignals->clearModel();
    stdOut = "Signals removed OK";       
    return true;
}

/*!
  List all signals from the target object.
 */
bool TasSignalPlugin::listSignals(void* objectInstance, QString ptrType, QString & stdOut)
{  
    QObject *target = castToObject(objectInstance, ptrType);
    bool result = false;
    if(target){
        TasDataModel* model = new TasDataModel();    
        TasObjectContainer& container = model->addNewObjectContainer(CONTAINER_ID.toInt(), "QtSignals", "QtSignals");

        for(int i = 0; i < target->metaObject()->methodCount(); ++i) {
            if (target->metaObject()->method(i).methodType() == QMetaMethod::Signal)
                container.addNewObject(QString::number(i), QString::fromLatin1(target->metaObject()->method(i).signature()), "QtSignal");
        }

        SerializeFilter* filter = new SerializeFilter();		    		
        filter->serializeDuplicates(true);		
        //filter is deleted by model
        QByteArray xml;
        model->serializeModel(xml, filter);    
        stdOut.append(QString::fromUtf8(xml.data()));
        delete model;
        result =  true;
    }
    else{
        stdOut = "No target object could be found.";
    }
    return result;
}

/*!
  Start listening to the given signal and report occurences to the model.
 */
bool TasSignalPlugin::enableSignal(void *objectInstance, QHash<QString, QString> parameters, QString & stdOut)
{
    bool result = false;
    if(parameters.contains(SIGNAL_KEY)){
        QString signalName = parameters.value(SIGNAL_KEY);
        QObject* target = castToObject(objectInstance, parameters.value(OBJECT_TYPE));
        //the object which we really want the changes related to the emitted signal
        QObject* owner = target;
        if(target && parameters[CHILD_SIGNAL] == "true"){
            owner = target;
            QObject* obj = target->findChild<QObject*>(parameters[CHILD_CLASS]);
            target = obj;            
        }
        if(target){
            int signalId = target->metaObject()->indexOfMethod(signalName.toLatin1().data());
            if(signalId != -1){
                const char* signature = target->metaObject()->method(signalId).signature();
                QString hashIdentificator = TasCoreUtils::objectId(target);
                TasObjectContainer* container = mOccuredSignals->findObjectContainer(CONTAINER_ID);
                if(!container){
                    container = &(mOccuredSignals->addNewObjectContainer(CONTAINER_ID.toInt(), "QtSignals", "QtSignals"));
                    if(parameters.contains(PROCESS_START_TIME)){
                        QString timeStamp = parameters.value(PROCESS_START_TIME);
                        TasObject& eventObj = container->addNewObject(TasCoreUtils::pointerId(&timeStamp), PROCESS_START_TIME, "event");
                        eventObj.addAttribute("timeStamp", parameters.value(PROCESS_START_TIME));
                    }
                }
                bool traverseSender = false;
                if(parameters[TRAVERSE_SENDER] == "true"){
                    traverseSender = true;
                }
                TasSignalSpy *tasSpy = new TasSignalSpy(target, (QString::number(QSIGNAL_CODE)+signature).toAscii().data(), 
                                                        *container, traverseSender);
                tasSpy->setTarget(owner);
                spyContainerHash.insert(QString(hashIdentificator + signature), tasSpy);                
                stdOut = "Enable signal called";
                result = true;
            }
            else{
                stdOut = "Signal " + signalName + " not found on object " + QString(target->metaObject()->className());
            }
        }
        else{
            stdOut = "No target object could be found!";
        }
    }
    else{
        stdOut = "No signal defined in parameters";
    }	
    return result;
}


QObject* TasSignalPlugin::castToObject(void* objectInstance, QString ptrType)
{
    QObject* target = 0;
    if( ptrType == WIDGET_TYPE ){     
        target = reinterpret_cast<QWidget*>(objectInstance);
    }
    else if( ptrType == GRAPHICS_ITEM_TYPE ){
        QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);   
        if(item && (item->isWidget() || item->isWindow())){
            target = (QObject*)((QGraphicsWidget*)item);
        }
    }
    else if( ptrType == APPLICATION_TYPE ){
        target = qApp;
    }
    return target;
}
