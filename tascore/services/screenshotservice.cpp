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



#include <QGraphicsItem>
#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QQuickWindow>
#include <QQuickItem>

#include "screenshotservice.h"
#include "taslogger.h"

/*!
  \class ScreenshotService
  \brief ScreenshotService closes the application

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
        QGuiApplication *app = qobject_cast<QGuiApplication*>(qApp);
        if (app) {
            getScreenshot(model, response);
        } else {
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
    QImage screenshot;
    QString pictureFormat = "PNG";
    while (i.hasNext()) {
        TasTarget* commandTarget = i.next();

        QString targetId = commandTarget->id();
        QString targetType = commandTarget->type();

        TasCommand* command = commandTarget->findCommand("Screenshot");

        // are required for command completion
        if (targetId.isEmpty() || targetType.isEmpty() || !command) {
            continue;
        }

        if (!command->parameter("format").isEmpty()) {
            pictureFormat = command->parameter("format");
        }

        if (!isFormatSupported(pictureFormat)) {
            errorMsg = "Given format " + pictureFormat + "is not supported. Supported formats are: PNG, JPEG and BMP.";
            break;
        }

        bool draw = (command->parameter("draw") == "true");

        QWidget* widget = 0;
        QQuickWindow* qtQuickWindow = 0;
        WId winId = 0;
        QRect rect(0,0,-1,-1);

        errorMsg = "Taking screenshot failed!";

        if (targetType == TYPE_GRAPHICS_VIEW) {
            QGraphicsItem* item = findGraphicsItem(targetId);

            if (item) {
                QGraphicsView* view = getViewForItem(item);
                if(view) {
                    ItemLocationDetails locationDetails = TestabilityUtils::getItemLocationDetails(item);
                    rect = QRect(locationDetails.windowPoint.x(),
                                 locationDetails.windowPoint.y(),
                                 locationDetails.width,
                                 locationDetails.height);

                    if (draw) {
                        widget = view->window();
                    } else {
                        winId = view->window()->winId();
                    }
                } else {
                    errorMsg = "Could not find a GraphicsView for the GraphicsItem!";
                }
            } else {
                errorMsg = "Could not find the GraphicsItem!";
            }
        } else if (targetType == TYPE_STANDARD_VIEW) {
            widget = findWidget(targetId);

            if (widget) {
                if ((widget->isWindow() && !draw) || widget->inherits("QDesktopWidget")) {
                    winId = widget->winId();
                    widget = 0;
                } else if (!draw) {
                    QPoint point = widget->mapToGlobal(QPoint(0,0));
                    QPoint windowPoint = widget->window()->mapFromGlobal(point);

                    rect = QRect(windowPoint.x(),
                                 windowPoint.y(),
                                 widget->rect().width(),
                                 widget->rect().width());
                    winId = widget->window()->winId();
                    widget = 0;
                }
            } else {
                TasLogger::logger()->debug("ScreenshotService::executeService application has no visible ui!");
                errorMsg = "Application has no visible ui!";
            }
        } else if (targetType == TYPE_QSCENEGRAPH) {
            QQuickItem* item = TestabilityUtils::findQuickItem(targetId);

            if (item) {
                QPointF offset = item->mapToScene(QPointF(0,0));
                rect = QRect(-offset.x(), -offset.y(), item->width(), item->height());
                qtQuickWindow = item->window();
            }
        } else {
            widget = getApplicationWidget();

            if (!widget) {
                QWindow *window = getApplicationWindow();
                //in case no window false, return the desktop
                qtQuickWindow = qobject_cast<QQuickWindow *>(window);

                if (!window) {
                    widget = qApp->desktop();
                } else {
                    TasLogger::logger()->warning("did find a window");
                }
            } else {
                TasLogger::logger()->warning("did not find widget");
            }
        }

        if (widget) {
            screenshot = widget->grab(rect).toImage();

            if (!screenshot.isNull()) {
                screenshot.setText("tas_id", objectId(widget));
            } else {
                TasLogger::logger()->warning("widget: screenshot was null");
            }
        } else if (qtQuickWindow) {
            screenshot = qtQuickWindow->screen()->grabWindow(qtQuickWindow->winId(), rect.x(), rect.y(), rect.width(), rect.height()).toImage();
            if (!screenshot.isNull()) {
                screenshot.setText("tas_id", objectId(qtQuickWindow));
            } else {
                screenshot = qtQuickWindow->grabWindow();
                if (screenshot.isNull()) {
                    TasLogger::logger()->warning("qtQuickWindow: screenshot was null");
                }
            }
        } else if (winId) {
            screenshot = QPixmap::grabWindow(winId, rect.x(), rect.y(), rect.width(), rect.height()).toImage();

            if (!screenshot.isNull()) {
                screenshot.setText("tas_id", QString::number(winId));
            } else {
                TasLogger::logger()->warning("winId: screenshot was null");
            }
        } else {
            TasLogger::logger()->warning("UNHANDLED SCREENSHOT CASE");
        }

        break;
    }

    if (!screenshot.isNull()) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        screenshot.save(&buffer, pictureFormat.toLatin1());
        response.setData(bytes);
        buffer.close();
    } else {
        TasLogger::logger()->warning(errorMsg);
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
