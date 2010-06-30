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
#include <QPoint>
#include <QRect>

#include "screenshotservice.h"
#include "taslogger.h"

/*!
  \class FixtureService
  \brief FixtureService closes the application

*/    

static QString NO_UI_ERROR = "Application has not ui, cannot take screenshot!";

ScreenshotService::ScreenshotService()
{
}

ScreenshotService::~ScreenshotService()
{
}

bool ScreenshotService::executeService(TasCommandModel& model, TasResponse& response)
{    
    if(model.service() == serviceName() ){
        TasLogger::logger()->debug("ScreenshotService::executeService in");
        if(qApp->type() != QApplication::Tty){
            getScreenshot(model, response);
        }        
        else{            
            TasLogger::logger()->debug("ScreenshotService::executeService application has no ui!");
            response.setErrorMessage(NO_UI_ERROR);
        }
        return true;
    }
    else{
        return false;
    }
}

void ScreenshotService::getScreenshot(TasCommandModel& model, TasResponse& response)
{
    QListIterator<TasTarget*> i(model.targetList());
    QString errorMsg = PARSE_ERROR;
    QPixmap screenshot;        
    QString pictureFormat = "PNG";
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();
        QString targetId = commandTarget->id();
        QString targetType = commandTarget->type();
        TasCommand* command = commandTarget->findCommand("Screenshot");
        //are required for command completion
        if(targetId.isEmpty() || targetType.isEmpty() || !command){
            continue;
        }

        if(!command->parameter("format").isEmpty()){
            pictureFormat = command->parameter("format");
        }

        if(!isFormatSupported(pictureFormat)){
            errorMsg = "Given format " + pictureFormat + "is not supported. Supported formats are: PNG, JPEG and BMP.";
            break;
        }

        bool draw = false;
        if(command->parameter("draw") == "true"){
            TasLogger::logger()->debug("Draw on!");
            draw = true;
        }

        quint32 id = targetId.toUInt();
        QWidget* target = 0;
        errorMsg = "Taking screenshot failed!";
        if(targetType == TYPE_GRAPHICS_VIEW){
            TasLogger::logger()->debug("TYPE_GRAPHICS_VIEW Target id:" + targetId);
            QGraphicsItem* item = findGraphicsItem(id); 
            if(item){
                QGraphicsView* view = getViewForItem(item);
                if(view){                    
                    QPoint point = view->mapFromScene(item->scenePos());
                    QPoint windowPoint = view->mapTo(view->window(), point);
                    QRectF itemRect = item->boundingRect();
                    ItemLocationDetails locationDetails = TestabilityUtils::getItemLocationDetails(item);
                    if(draw){
                        
                        screenshot = QPixmap::grabWidget(view->window(), locationDetails.scenePoint.x(), locationDetails.scenePoint.y(),
                                                         locationDetails.width, locationDetails.height);
                    }                 
                    else{
                        screenshot = QPixmap::grabWindow(view->window()->winId(), locationDetails.scenePoint.x(), locationDetails.scenePoint.y(),
                                                         locationDetails.width, locationDetails.height);
                    }
                }
                else{
                    errorMsg = "Could not find a GraphicsView for the GraphicsItem!";
                }
            }
            else{
                errorMsg = "Could not find the GraphicsItem!";
            }
        }
        else{
            if(targetType == TYPE_STANDARD_VIEW){
                TasLogger::logger()->debug("TYPE_STANDARD_VIEW about to find widget Target id:" + targetId);
                target = findWidget(id);
            }
            else{
                TasLogger::logger()->debug("TYPE_APPLICATION_VIEW about to find application window Target id:" + targetId);
                target = getApplicationWindow();
                //in case no window false, return the desktop
                if(!target){
                    draw = false;
                    target = qApp->desktop();
                }
            }
            if(target){
                if ( (target->isWindow() && !draw) || target->inherits("QDesktopWidget")){    
                    screenshot = QPixmap::grabWindow(target->winId());
                }
                else{
                    if(draw){
                        screenshot = QPixmap::grabWidget(target);
                    }
                    else{
                        QPoint point = target->mapToGlobal(QPoint(0,0));
                        QPoint windowPoint = target->window()->mapFromGlobal(point);
                        screenshot = QPixmap::grabWindow(target->window()->winId(), windowPoint.x(), windowPoint.y(), 
                                                         target->rect().width(), target->rect().height());
                    }
                }
            }
            else{
                TasLogger::logger()->debug("ScreenshotService::executeService application has no visible ui!");
                errorMsg = "Application has no visible ui!";
            }
        }
        break;        
    }
    
    if (!screenshot.isNull()){        
        QByteArray* bytes = new QByteArray();     
        QBuffer buffer(bytes);
        buffer.open(QIODevice::WriteOnly);
        screenshot.save(&buffer, pictureFormat.toAscii());
        response.setData(bytes);
    }
    else{
        response.setErrorMessage(errorMsg);
    }

}

bool ScreenshotService::isFormatSupported(QString format)
{
    if( QString::compare(format, "PNG", Qt::CaseInsensitive) == 0 ||
        QString::compare(format, "JPEG", Qt::CaseInsensitive) == 0 ||
        QString::compare(format, "BMP", Qt::CaseInsensitive) == 0){
        return true;        
    }
    else{
        return false;
    }
}
