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
#include <QQuickView>
#include <QtPlugin>
#include <QDebug>
#include <QHash>

#include <taslogger.h>
#include <testabilityutils.h>
#include <tasqtdatamodel.h>
#include "fpsfixture.h"


/*!
  \class FpsFixture
  \brief Collects frames per second data from widgets.
        
  Filters paint events to the given target and calculates fps.
  For graphicsitems the view's viewport is used.
*/

/*!
  Constructor
*/
FpsFixture::FpsFixture(QObject* parent)
    :QObject(parent)
{}

/*!
  Destructor
*/
FpsFixture::~FpsFixture()
{
    QMutableHashIterator<QObject*,  FspMeasurer*> i(mFpsCounters);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    mFpsCounters.clear();
}

/*!
  Implementation for traverse so always true.
*/
bool FpsFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    // place your own code below
    bool result = true;

    QObject* target = getTarget(objectInstance, parameters.value(OBJECT_TYPE));
    if(target){
        TasLogger::logger()->debug("FpsFixture::execute " + actionName);
        if(actionName == "startFps"){
            if(mFpsCounters.contains(target)){
                mFpsCounters.value(target)->startFpsMeasure();
            }
            else{
                mFpsCounters.insert(target, new FspMeasurer(target));
            }
        }
        else if ( actionName == "collectData" || actionName == "stopFps"){
            if(mFpsCounters.contains(target)){
                FspMeasurer* fpsM = mFpsCounters.value(target);
                //make model
                printFpsResults(fpsM->collectedData(), target, stdOut);
                //restart collecting or stop 
                if (actionName == "stopFps"){
                    fpsM->stopFpsMeasure();
                }
                else{
                    fpsM->restartFpsMeasure();
                }
            }
            else{
                result = false;
                stdOut = "Fps data collection was never initiated for given target.";
            }
        }
        else{
            result = false;
            stdOut = "Unknown action for fpsFixture.!";
        }
    }
    else{
        result = false;
        stdOut = "Unknown target!";
    }

    return result;
}

void FpsFixture::printFpsResults(QList< QPair<QString,int> > fpsData, QObject* target, QString& stdOut)
{
    TasDataModel* model = new TasDataModel();

    //TasObjectContainer& container = model->addNewObjectContainer((int)model, "QtFps", "QtFps");
    TasObjectContainer& container = model->addNewObjectContainer(1, "QtFps", "QtFps");

    QString name = TestabilityUtils::getApplicationName();

    TasObject& application = container.addNewObject(QString::number(qApp->applicationPid()), name, "application");
    application.addAttribute("exepath", qApp->applicationFilePath().toLatin1().data());
    application.addAttribute("FullName", qApp->applicationFilePath().toLatin1().data());
    application.addAttribute("dirpath", qApp->applicationDirPath().toLatin1().data());

    QString objectType = target->metaObject()->className();
    objectType.replace(QString(":"), QString("_"));
    TasObject& targetObj = application.addNewObject(TasCoreUtils::objectId(target), target->objectName(), objectType);

    TasObject& fpsObj = targetObj.addNewObject(TasCoreUtils::pointerId(&fpsData),"FpsResults", "results");
    fpsObj.addAttribute("count", QString::number(fpsData.size()));
    for(int i = 0 ; i < fpsData.size(); i++ ){        
        TasObject& fpsD = fpsObj.addNewObject(QString::number(i),"FpsData", "fps");     
        QPair<QString,int> value = fpsData.at(i);
        fpsD.addAttribute("timeStamp", value.first);
        fpsD.addAttribute("frameCount", value.second);
    }
    QByteArray xml;
    model->serializeModel(xml);    
    stdOut.append(QString::fromUtf8(xml.data()));
    delete model;
}

/*!
  Find the target for the fps measurement.
 */
QObject* FpsFixture::getTarget(void* objectInstance, QString objectType)
{
    QObject* target = 0;
    if(objectType == WIDGET_TYPE ){
        QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
        if (widget) {
            //if graphicsview, then paint events go to viewport 
            //make that the target (as with graphicsitems)
            QGraphicsView* view = qobject_cast<QGraphicsView*>(widget);
            if(view){
                if(view->viewport()){
                    widget = view->viewport();
                }
            }
            target = qobject_cast<QObject*>(widget);
        }
    }
    else if(objectType == GRAPHICS_ITEM_TYPE ){
        QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);
        if(item) {
            QPoint point;
            QWidget* viewPort = TestabilityUtils::viewPortAndPosition(item, point);
            target = qobject_cast<QObject*>(viewPort);
        }
    }
    else if(objectType == QQUICKVIEW_TYPE) {
        QQuickView* view = reinterpret_cast<QQuickView*>(objectInstance);
        if (view) {
            target = qobject_cast<QObject*>(view);
        }
    }   
    return target;
}


/*!
  Constructs a fps measuser for the given target object.
  Ownership of the object is not assumed.
  Will also start the measurement.
 */
FspMeasurer::FspMeasurer(QObject *target)
{
    mTarget = target;
    startFpsMeasure();

}

FspMeasurer::~FspMeasurer()
{
    mTarget = 0;
    mFpsValues.clear();
}

QList< QPair<QString,int> > FspMeasurer::collectedData()
{
    //add the last one also
    mFpsValues.append(QPair<QString, int>(mTimeStamp.toString("hh:mm:ss.zzz"), mFpsCounter)); 
    return mFpsValues;
}

void FspMeasurer::startFpsMeasure()
{
    mFpsValues.clear();
    mFpsCounter = 0;

    if(mTarget->isWidgetType()) {
        mTarget->installEventFilter(this);
    } else if(mTarget->isWindowType()) {
        QQuickView* view = qobject_cast<QQuickView*>(mTarget);
	view->connect(view, SIGNAL(frameSwapped()),
                         this, SLOT(collectFps()), Qt::DirectConnection);
    }
}

void FspMeasurer::restartFpsMeasure()
{
    mFpsValues.clear();
    mFpsCounter = 0;
}

void FspMeasurer::stopFpsMeasure()
{
    restartFpsMeasure();
    
    if(mTarget->isWidgetType()) {
        mTarget->removeEventFilter(this);
    } else if(mTarget->isWindowType()) {
	QQuickView* view = qobject_cast<QQuickView*>(mTarget);
        view->disconnect(view, SIGNAL(frameSwapped()),
                         this, SLOT(collectFps()));
    }
}

void FspMeasurer::collectFps()
{
    //start collecting
    if(mFpsCounter == 0){
        mTimeStamp = QTime::currentTime();
        mTimer.start();
        mFpsCounter++;
    }
    else{
        if(mTimer.elapsed() < 1000){
            mFpsCounter++;
        }
        else{
            mFpsValues.append(QPair<QString,int>(mTimeStamp.toString("hh:mm:ss.zzz"), mFpsCounter)); 
            mFpsCounter = 1;
            mTimer.restart();
            mTimeStamp = QTime::currentTime();
        }
    }  
}

bool FspMeasurer::eventFilter(QObject *target, QEvent *event)
{   
    Q_UNUSED(target);
    if( event->type() == QEvent::Paint){
    	collectFps();
	}
    return false;
}
