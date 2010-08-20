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
#include <QtPlugin>
#include <QDebug>
#include <QHash>
#include <QTimer>

#include <taslogger.h>
#include <testabilityutils.h>
#include <tasqtdatamodel.h>
#include <tastraverserloader.h>

#include "dialogfixture.h"


static const QString CLASS_NAME = "className";
static const QString INTERVAL = "interval";


Q_EXPORT_PLUGIN2(dialogfixture, DialogFixture)


/*!
  \class DialogFixture
  \brief Listens to events to detect a dialog
        
*/

/*!
  Constructor
*/
DialogFixture::DialogFixture(QObject* parent)
    :QObject(parent)
{
    mFiltering = false;
    TasTraverserLoader loader;
    mTraverser = new TasUiTraverser(loader.loadTraversers());
}

/*!
  Destructor
*/
DialogFixture::~DialogFixture()
{
    delete mTraverser;
}

/*!
  Implementation for traverse so always true.
*/
bool DialogFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    Q_UNUSED(objectInstance);
    TasLogger::logger()->debug("DialogFixture::execute");
    bool result = true;

    if(actionName == "waitDialog"){
        QString className = parameters[CLASS_NAME].trimmed();
        int interval = parameters[INTERVAL].toInt();
        TasLogger::logger()->debug("DialogFixture::execute waitdialog for " + className);
        startDialogDetection(className, interval);
    }
    else if(actionName == "printDialog"){
        QString className = parameters[CLASS_NAME].trimmed();
        TasLogger::logger()->debug("DialogFixture::execute get data for " + className);
        if(mDialogData.contains(className)){
            TasLogger::logger()->debug("DialogFixture::execute data found serialize model.");
            TasDataModel *model = mDialogData.value(className);
            mDialogData.remove(className);

            QByteArray xml;
            model->serializeModel(xml);    
            stdOut.append(QString::fromUtf8(xml.data()));
            delete model;

            mClassNames.remove(className);
        }
        else{
            TasLogger::logger()->debug("DialogFixture::execute no data for item.");
            result = false;
            stdOut = "No data for the given class name. Maybe widget was not shown!";
        }

        if(mClassNames.isEmpty()){
            qApp->removeEventFilter(this);
            mFiltering = false;
        }

    }
    else{
        stdOut = "Uknown action for DialogFixture. Make sure action name is correct or that you are using the correct fixture.";
        result = false;
    }

    return result;
}

void DialogFixture::startDialogDetection(const QString& className, int interval)
{
    TasLogger::logger()->debug("DialogFixture::startDialogDetection");
    mClassNames.insert(className, interval);
    if(!mFiltering){
        qApp->installEventFilter(this);
        mFiltering = true;
    }
}


bool DialogFixture::eventFilter(QObject *target, QEvent *event)
{
    if( event->type() == QEvent::Show && target ){
        TasLogger::logger()->debug("DialogFixture::eventFilter");
        QHashIterator<QString, int> i(mClassNames);
        while (i.hasNext()) {
            i.next();
            QString className = i.key();            
            if(target->inherits(className.toAscii())){
                DialogTimer* timer = new DialogTimer(className, mClassNames.value(className));
                connect(timer, SIGNAL(traverseState(const QString&)), this, SLOT(traverse(const QString&)));
                mClassNames.remove(className);
                break;
            }
        }
    }
    return false;
}

void DialogFixture::traverse(const QString& className)
{
    //no double traverse
    if(!mDialogData.contains(className)){
        TasLogger::logger()->debug("DialogFixture::traverse for " + className);
        TasDataModel* model = mTraverser->getUiState(NULL); // No command available?
        mDialogData.insert(className, model);
    }
}
    
DialogTimer::DialogTimer(const QString className, int interval)
{
    mClassName = className;
    QTimer::singleShot(interval, this, SLOT(timeout()));
}

void DialogTimer::timeout()
{
    emit traverseState(mClassName);
    deleteLater();
}
