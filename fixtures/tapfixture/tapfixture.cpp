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



#include <QGraphicsWidget>
#include <QMouseEvent>
#include "tapfixture.h"
#include "testabilityutils.h"
#include "taslogger.h"


/*!
  \class TapFixturePlugin
  \brief
*/

TapFixturePlugin::TapFixturePlugin(QObject* parent) :QObject(parent) {}
TapFixturePlugin::~TapFixturePlugin() {}

bool TapFixturePlugin::execute(
        void* objectInstance,
        QString actionName,
        QHash<QString, QString> parameters,
        QString& stdOut)
{

    Q_UNUSED(stdOut);

    TasLogger::logger()->debug("> TapFixturePlugin::execute");
    bool result = true;

    if (actionName == "tap") {
        TasLogger::logger()->debug("TapFixturePlugin::execute Tapping...");
        Qt::MouseButton button = Qt::LeftButton;
        if (parameters.value(OBJECT_TYPE) == WIDGET_TYPE) {
            TasLogger::logger()->debug("TapFixturePlugin::execute ...widget");
            QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
            if (widget) {
                QPoint widgetCenter(widget->width()/2, widget->height()/2);
                QMouseEvent* eventPress = new QMouseEvent(QEvent::MouseButtonPress, widgetCenter, button, button, 0);
                qApp->sendEvent(widget, eventPress);
                QMouseEvent* eventRelease = new QMouseEvent(QEvent::MouseButtonRelease, widgetCenter, button, button, 0);
                qApp->sendEvent(widget, eventRelease);
            }
        }
        else if (parameters.value(OBJECT_TYPE) == GRAPHICS_ITEM_TYPE) {
            TasLogger::logger()->debug("TapFixturePlugin::execute ...graphics item");
            QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);
            if (item) {
                // To be able to grab the mouse, we'll lie being visible
                bool itemWasVisible = item->isVisible();
                if (!item->isVisible()) {
                    item->setVisible(true);
                }
                item->grabMouse();

                // Find this item's view for getting a widget
                QGraphicsScene* scene = item->scene();
                QList<QGraphicsView*> list = scene->views();
                QGraphicsView* match = 0;
                foreach (QGraphicsView* view, scene->views()) {
                    if (view->items().indexOf(item) != -1) {
                        match = view;
                        break;
                    }
                }

                // Get the widget for sending mouse events to
                QWidget* widget = 0;
                if (match) {
                    widget = match->viewport();
                }
                if (!widget) {
                    return result;
                }

                // Press and release in the center of the item
                qreal itemWidth = item->boundingRect().width();
                qreal itemHeight = item->boundingRect().height();
                QPoint itemCenter((int) itemWidth / 2, (int) itemHeight / 2);
                QMouseEvent* eventPress = new QMouseEvent(QEvent::MouseButtonPress, itemCenter, button, button, 0);
                qApp->sendEvent(widget, eventPress);
                QMouseEvent* eventRelease = new QMouseEvent(QEvent::MouseButtonRelease, itemCenter, button, button, 0);
                qApp->sendEvent(widget, eventRelease);

                // Release the mouse and reset visibility
                item->ungrabMouse();
                if (itemWasVisible == false) {
                    item->setVisible(false);
                }
            }
        }
    }
    TasLogger::logger()->debug("< TapFixturePlugin::execute");
    return result;
}
