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
 
 

#ifndef WEBKITTRAVERSE_H
#define WEBKITTRAVERSE_H

#include <QObject>

#if QT_VERSION >= 0x040600

#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebView>

#endif

#include <tasqtdatamodel.h>
#include <tastraverseutils.h>

#include "tastraverseinterface.h"

class QGraphicsProxyWidget;
class TasCommand;
class QGraphicsWebView;

class WebKitTraverse :  public QObject, public TasTraverseInterface
 {
 Q_OBJECT
 Q_INTERFACES(TasTraverseInterface)
 
public:
     WebKitTraverse(QObject* parent=0);
     ~WebKitTraverse();

     void traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command);
     void traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command);     
	 void beginTraverse(TasCommand* command);
	 void endTraverse();
    

private:

#if QT_VERSION >= 0x040600
    void traverseWebElement(TasObject* parent, QPoint parentPos, QPoint screenPos, QWebElement* webElement, const QString& webFrameId);
    void traverseQWebView(TasObject* objectInfo, QWebView* webView);
    void traverseQWebPage(TasObject& pageInfo, QWebPage* webPage, const QPoint& webViewPos, const QPoint& screenPos);

    void traverseQGraphicsWebView(TasObject* objectInfo, QGraphicsWebView* webView, TasCommand* command);

    void traverseFrame(QWebFrame * webFrame, TasObject& parent, QString parentId, const QPoint& parentPos, const QPoint& screenPos);

//    void parseAttributes(QWebElement* webElement, TasObject* objInfo); // deprecated

    void parseAttributes(QWebElement* webElement, TasObject& objInfo);

    QString parseElementText(QString innerXml);

    void addAttribute( TasObject& object, const QString &name, QWebPage* webPage );
    void addAttribute( TasObject& object, const QString &name, const QPoint& point );
    void addAttribute( TasObject& object, const QString &name, QWebElement* webElement );
    void addAttribute( TasObject& object, const QString &name, QWebFrame* webFrame );

    void addAttribute( TasObject& object, const QString &name, int value );
    void addAttribute( TasObject& object, const QString& name, const QString& string );
    void addAttribute( TasObject& object, const QString &name, bool value );

    QGraphicsWebView* mTemporaryPointerToQGraphicsWebView;

#endif

private:
	 TasTraverseUtils* mTraverseUtils;
    
 };

#endif
