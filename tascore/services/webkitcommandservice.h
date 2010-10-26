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
 


#ifndef WEBKITCOMMANDSERVICE_H
#define WEBKITCOMMANDSERVICE_H

#include <QObject>
#include <QTimer>
#include <QQueue>
#include <QList>
#include <QtWebKit/QWebFrame>

#include "tasservicebase.h"
#include "testabilityutils.h"

class WebkitCommandService : public TasServiceBase
{
public:
    WebkitCommandService();
	~WebkitCommandService();

	/*!
	  From ServiceInterface
	*/
	bool executeService(TasCommandModel& model, TasResponse& response);

	QString serviceName()const { return WEBKIT_COMMAND; }

private:
    bool scrollQWebFrame(TasTarget* target, TasCommand* command);
    bool traverserScrollToQWebFrame(QWebFrame* webFrame, QString id, int dx, int dy);

    bool executeJavaScriptWebElement(TasTarget* target, TasCommand* command,TasResponse& response);
    bool executeJavaScriptQWebFrame(TasTarget* target, TasCommand* command,TasResponse& response);

    bool traverseJavaScriptToQWebFrame(QWebFrame* webFrame, QString javaScript, QString id,TasResponse& response);
    bool traverseJavaScriptToWebElement(TasResponse& response, QWebFrame* webFrame, QString webFrameId, QString javaScript, QString query, int &index, TasCommand* command,int parentFrames=0);


    bool executeJavascriptOnWebElement(QWebFrame* webFrame, QString webFrameId, QString javaScript, QString elementId,TasResponse& response);
    QWebElement lookForWebElement(const QWebElement &parentElement, QString elementId, QString webFrameId);
    QWebFrame* lookForWebFrame(QWebFrame* webFrame, QString webFrameId);

    QList<QWebFrame*> traverseStart();
    QList<QWebFrame*> traverseObject(QObject* object);
    QList<QWebFrame*> traverseGraphicsItem(QGraphicsItem* graphicsItem);
    QList<QWebFrame*> traverseGraphicsItemList(QGraphicsItem* graphicsItem);
    QList<QWebFrame*> traverseGraphicsViewItems(QGraphicsView* view);

    QList<QWebElement> traverseFrame(QWebFrame* webFrame, const QPoint& parentPos, const QPoint& screenPos, QHash<QString, QString> attributeMatchHash);
    QList<QWebElement> traverseWebElement(QPoint parentPos, QPoint screenPos,QWebElement* webElement,QHash<QString, QString> attributeMatchHash);

    QString parseElementText(QString innerXml);

private:
    QString mErrorMessage;
    int counter;
};

#endif
