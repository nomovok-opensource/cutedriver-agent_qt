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
 

#include <QtPlugin>
#include <QDebug>

#if QT_VERSION >= 0x040600

#include <QtWebKit/QWebView>
#include <QtWebKit/QGraphicsWebView>
#include <QtWebKit/QWebPage>
#include <QtWebKit/QWebFrame>


#endif

#include <QGraphicsWidget>

#include "webkittraverse.h"
#include "taslogger.h"
#include "tasqtcommandmodel.h"
#include <testabilityutils.h>

Q_EXPORT_PLUGIN2(webkittraverse, WebKitTraverse)

/*!
    \class WebKitTraverse
    \brief WebKitTraverse traverse web kit
        
*/

/*!
    Constructor
*/
WebKitTraverse::WebKitTraverse(QObject* parent)
    :QObject(parent), counter(0)
{
}

/*!
    Destructor
*/
WebKitTraverse::~WebKitTraverse()
{}

/*!
  Traverse graphicsitem(widget) for web kit
*/
void WebKitTraverse::traverseGraphicsItem(TasObject* objectInfo, QGraphicsItem* graphicsItem, TasCommand* command)
{
    Q_UNUSED(objectInfo);
    Q_UNUSED(graphicsItem);
    Q_UNUSED(command);       
}



/*!
  Traverse object(widget) for web kit
*/
void WebKitTraverse::traverseObject(TasObject* objectInfo, QObject* object, TasCommand* command)
{    

    //TasLogger::logger()->debug("WebKitTaverse::traverseObject");
#if QT_VERSION >= 0x040600
    if(object->inherits("QWebView")){
        TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebView");
        QWebView* webView = qobject_cast<QWebView*>(object);               
        if(webView){
            traverseQWebView(objectInfo, webView);
        }        
    }

    if(object->inherits("QGraphicsWebView")){
        TasLogger::logger()->debug("WebKitTaverse::traverseObject QGraphicsWebView");
        QGraphicsWebView* webView = qobject_cast<QGraphicsWebView*>(object);               
        if(webView){
            traverseQGraphicsWebView(objectInfo, webView, command);
        }        
    }




    if (object->inherits("DuiApplication")) {
        TasLogger::logger()->debug("WebKitTaverse::traverseObject DuiApplication.");
        
        QVariant angelPid = object->property("angelPid");
        
        if (angelPid.isValid()) {
            int pid = angelPid.toInt();
            TasLogger::logger()->debug("WebKitTaverse::traverseObject Actually an angel pid.");
                    TasObject& obj = objectInfo->addObject();            
                    obj.setId(0);
                    obj.setType("TDriverRef");    
                    obj.setName("TDriverRef");
                    TasLogger::logger()->debug("WebKit::traverseObject referring to " + 
                                               QString::number(pid));
                    
                    obj.addAttribute("uri", QString::number(pid));

            
            return;
        }



        QList<QObject*> list = object->findChildren<QObject*>();
        TasLogger::logger()->debug(">>>> Found : " + QString::number(list.size())  + " children");
        foreach (QObject* obj, list) {
            const QMetaObject* mObj = obj->metaObject();
            TasLogger::logger()->debug(">>>> Found : " + QString(mObj->className()));


            if (QString(mObj->className()) == "WRT::Maemo::WebAppletRunner") {
                TasLogger::logger()->debug("Found communication server");
                QGraphicsWebView* view = 0;
                TasLogger::logger()->debug("Invoking method for communicationserver");
                
                QMetaObject::invokeMethod(obj, "view",
                                          Qt::DirectConnection,
                                          Q_RETURN_ARG(QGraphicsWebView*, view));
                if (view) {
                    TasLogger::logger()->debug("Traversing webpage");
                    TasObject& obj = objectInfo->addObject();        
                    QPoint p;
                    // Retrieve parent xy coords
                    TasLogger::logger()->debug("Trying to retrieve parent xy");
                    QString xpos = command->parameter("x_parent_absolute");
                    QString ypos = command->parameter("y_parent_absolute");
                    if (!xpos.isEmpty() && !ypos.isEmpty()) {
                        TasLogger::logger()->debug("XY: " + xpos + " " + ypos);

                        p = QPoint(xpos.toInt(), ypos.toInt());
                    }
                    traverseQWebPage(obj,view->page(), p, p);
                } else {
                    TasLogger::logger()->debug("Page instance not found");
                }
                
            }

                                       
        }
    }

    // support for Symbian CWRT 9.2 and 10.1 - Fullscreen mode only
    if (object->inherits("WRT__WrtWebView")) {
        TasLogger::logger()->debug("WebKitTaverse::traverseObject WRT__WrtWebView");
        
        QGraphicsWebView* view = 0;    
        QMetaObject::invokeMethod(object, "view",
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(QGraphicsWebView*, view));
                
        if (view) {
          TasLogger::logger()->debug("Traversing webpage");
          TasObject& object = objectInfo->addObject();        
          QPoint p;
          // Retrieve parent xy coords
          TasLogger::logger()->debug("Trying to retrieve parent xy");
          QString xpos = "0"; //command->parameter("x_parent_absolute");
          QString ypos = "0"; //command->parameter("y_parent_absolute");
          if (!xpos.isEmpty() && !ypos.isEmpty()) {
            TasLogger::logger()->debug("XY: " + xpos + " " + ypos);

            p = QPoint(xpos.toInt(), ypos.toInt());
          }
          traverseQWebPage(object,view->page(), p, p);
        } else {
          TasLogger::logger()->debug("Page instance not found");
        }
                
     }

    if (object->inherits("GVA::PopupWebChromeItem")) {
//        TasLogger::logger()->debug(" WebKitTaverse::traverseObject found " + QString(object->metaObject()->className()) );

        QWebElement element;
        QMetaObject::invokeMethod(object, "element",
                                  Qt::AutoConnection,
                                  Q_RETURN_ARG(QWebElement, element));
        if (!element.isNull()) {
//          TasLogger::logger()->debug("Traversing webelement");
          TasObject& tas_object = objectInfo->addObject();

          QGraphicsWidget* item = qobject_cast<QGraphicsWidget*>(object);
          QPoint p(item->pos().x()-element.geometry().x(),item->pos().y()-element.geometry().y());

          QString tasId = TasCoreUtils::objectId(object);

          traverseWebElement(&tas_object,p,p,&element, tasId);
        } else {
//          TasLogger::logger()->debug("QWebElement not found");
        }

     }


#endif
}

#if QT_VERSION >= 0x040600


// There's no common base clas..
void WebKitTraverse::traverseQGraphicsWebView(TasObject* objectInfo, QGraphicsWebView* webView, TasCommand* command) 
{
    counter = 0;

    objectInfo->setType("QGraphicsWebView");
    QWebPage* webPage = webView->page();
//    QPoint webViewPos = webView->mapTo(webView->window(), QPoint(0, 0));


    QPair<QPoint,QPoint>  coords = addGraphicsItemCoordinates(objectInfo, webView, command);
    if(webPage) {
        //TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebPage != null");
        TasObject& pageInfo = objectInfo->addObject();
        traverseQWebPage(pageInfo, webPage, coords.first,coords.second);
    }
    
    







//     QPoint screenPos = webView->mapToGlobal(QPoint(0, 0));
//     QGraphicsProxyWidget* proxy = TestabilityUtils::parentProxy(webView);
//     if (proxy) {
//         // If the webview is inside a graphics proxy, 
//         // Take the proxy widget global position
//         screenPos = TestabilityUtils::proxyCoordinates(proxy);
//         // Also move the webview pos to the proxy pos
//         webViewPos += TestabilityUtils::proxyCoordinates(proxy, false);
//     }


    
}

void WebKitTraverse::traverseQWebView(TasObject* objectInfo, QWebView* webView) 
{    
    counter = 0;
//    TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebView != null");
    objectInfo->setType("QWebView");    

    QWebPage* webPage = webView->page();
    QPoint webViewPos = webView->mapTo(webView->window(), QPoint(0, 0));

    QPoint screenPos = webView->mapToGlobal(QPoint(0, 0));
    QGraphicsProxyWidget* proxy = TestabilityUtils::parentProxy(webView);
    if (proxy) {
        // If the webview is inside a graphics proxy, 
        // Take the proxy widget global position
        screenPos = TestabilityUtils::proxyCoordinates(proxy);
        // Also move the webview pos to the proxy pos
        webViewPos += TestabilityUtils::proxyCoordinates(proxy, false);
    }


    if(webPage) {
        //TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebPage != null");
        TasObject& pageInfo = objectInfo->addObject();
        traverseQWebPage(pageInfo, webPage, webViewPos,screenPos);
    }

}


void WebKitTraverse::traverseQWebPage(TasObject& pageInfo, QWebPage* webPage, 
                                      const QPoint& webViewPos, const QPoint& screenPos) 
{
    pageInfo.setId(TasCoreUtils::objectId(webPage));
    pageInfo.setType("QWebPage");    
    //        int parentId = (int)webView;
//        pageInfo.setParentId(parentId);        
    pageInfo.setName("QWebPage");
    pageInfo.addBooleanAttribute("isModified", webPage->isModified());
    pageInfo.addAttribute("totalBytes", QString::number(webPage->totalBytes()));
    pageInfo.addAttribute("receivedBytes", QString::number(webPage->bytesReceived()));                        
    pageInfo.addAttribute("selectedText", webPage->selectedText());            
    pageInfo.addAttribute("viewPortWidth", webPage->viewportSize().width());
    pageInfo.addAttribute("viewPortHeight", webPage->viewportSize().height());                        
    pageInfo.addBooleanAttribute("isContentEditable", webPage->isContentEditable());
    pageInfo.addAttribute("x_absolute", screenPos.x());                        
    pageInfo.addAttribute("y_absolute", screenPos.y());
    pageInfo.addAttribute("x", webViewPos.x());                        
    pageInfo.addAttribute("y", webViewPos.y());

//         pageInfo.addAttribute("width", webView->width());                        
//         pageInfo.addAttribute("height", webView->height());


    traverseFrame(webPage->mainFrame(), pageInfo, TasCoreUtils::objectId(webPage), webViewPos, screenPos);
/*
        if(mainFrame) {            
            //TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebFrame != null");
            //TasLogger::logger()->debug("WebKitTaverse::traverseObject " + mainFrame->renderTreeDump());    
                    
            TasObject& frameInfo = pageInfo.addObject();
            int tasId = (int)mainFrame;
            frameInfo.setId(tasId);
            frameInfo.setType("QWebFrame");    
            int parentId = (int)webPage;
            frameInfo.setParentId(parentId);        
            frameInfo.setName(mainFrame->frameName());
            frameInfo.addAttribute("title", mainFrame->title());
            frameInfo.addAttribute("url", mainFrame->url().toString());
            frameInfo.addAttribute("requestedUrl", mainFrame->requestedUrl().toString());
            frameInfo.addAttribute("baseUrl", mainFrame->baseUrl().toString());
            frameInfo.addAttribute("x", webViewPos.x());                        
            frameInfo.addAttribute("y", webViewPos.y());
            frameInfo.addAttribute("x_absolute", screenPos.x());                        
            frameInfo.addAttribute("y_absolute", screenPos.y());
//             frameInfo.addAttribute("width", webView->width());                        
//             frameInfo.addAttribute("height", webView->height());

            QWebElement docElement = mainFrame->documentElement();
            traverseWebElement(&frameInfo, webViewPos-mainFrame->scrollPosition(), screenPos-mainFrame->scrollPosition(), &docElement);
        }
*/
}

void WebKitTraverse::traverseFrame(QWebFrame* webFrame, TasObject& parent, QString parentId, const QPoint& parentPos, const QPoint& screenPos)
{
    if(webFrame) {            
//        TasLogger::logger()->debug("WebKitTaverse::traverseFrame webFrame != null");
        //TasLogger::logger()->debug("WebKitTaverse::traverseObject " + webFrame->renderTreeDump());    
                    
        QString tasId = TasCoreUtils::objectId(webFrame);
        TasObject& frameInfo = parent.addObject();
        frameInfo.setId(tasId);
        frameInfo.setType("QWebFrame");    
        frameInfo.setParentId(parentId);        
        frameInfo.setName(webFrame->frameName());
        frameInfo.addAttribute("title", webFrame->title());
        frameInfo.addAttribute("url", webFrame->url().toString());
        frameInfo.addAttribute("requestedUrl", webFrame->requestedUrl().toString());
        frameInfo.addAttribute("baseUrl", webFrame->baseUrl().toString());
        frameInfo.addAttribute("x", parentPos.x() + webFrame->pos().x());                        
        frameInfo.addAttribute("y", parentPos.y() + webFrame->pos().y());
        frameInfo.addAttribute("x_absolute", screenPos.x() + webFrame->pos().x());                        
        frameInfo.addAttribute("y_absolute", screenPos.y() + webFrame->pos().y());
        frameInfo.addAttribute("width", webFrame->geometry().width());
        frameInfo.addAttribute("height", webFrame->geometry().height());
        frameInfo.addAttribute("objectType", "Web");

        QWebElement docElement = webFrame->documentElement();
        traverseWebElement(&frameInfo, parentPos+webFrame->pos()-webFrame->scrollPosition(), screenPos+webFrame->pos()-webFrame->scrollPosition(), &docElement, tasId);

        // find all direct children frames and traverse those too
        QList<QWebFrame*> frames = webFrame->childFrames();
        for(int i = 0; i < frames.size(); i++) {
            traverseFrame(frames[i], frameInfo, tasId, parentPos-webFrame->scrollPosition(), screenPos-webFrame->scrollPosition());
        }
   }

}
 
/*!
  Traverse QWebElement
*/
void WebKitTraverse::traverseWebElement(TasObject* parent, QPoint parentPos, QPoint screenPos, QWebElement* webElement, const QString& webFrameId)
{
    if(webElement == NULL || webElement->isNull()) {
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement webElement is null");
        return;
    }    

    // traverse this element and all children
    //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement traverse this element:" + webElement->tagName());
    TasObject& childInfo = parent->addObject();
    //childInfo.setId(QString::number(++counter));    
    uint elementId = qHash(webElement->toOuterXml()+webFrameId);
    childInfo.setId(QString::number(elementId));
    childInfo.setType(webElement->tagName().toLower());    
    //QWebElement parentElement = webElement->parent();
    //int parentId = (int)&parentElement;
    //childInfo.setParentId(parentId);        
    childInfo.setName(webElement->localName().toLower());
    childInfo.addAttribute("webFrame",webFrameId);
    // position need to be relative to parent
    // NOTE all of these needs to be handled in webkitcommandservice.cpp
    QPoint childPos = QPoint(webElement->geometry().x(), webElement->geometry().y());
    childInfo.addAttribute("x", childPos.x()+parentPos.x());
    childInfo.addAttribute("y", childPos.y()+parentPos.y());   
    childInfo.addAttribute("x_absolute", childPos.x()+screenPos.x());
    childInfo.addAttribute("y_absolute", childPos.y()+screenPos.y());   
    childInfo.addAttribute("width", webElement->geometry().width());
    childInfo.addAttribute("height", webElement->geometry().height());
    childInfo.addAttribute("objectType", "Web");
    childInfo.addBooleanAttribute("visible", webElement->styleProperty("visibility", QWebElement::ComputedStyle).toLower() == "visible");
    childInfo.addBooleanAttribute("hasFocus", webElement->hasFocus());

    if(webElement->hasAttributes()) {
        parseAttributes(webElement, &childInfo);
    }

    // traverse first child
    QWebElement firstChild = webElement->firstChild();

    childInfo.addAttribute("innerText", webElement->toPlainText());
    //webElement->
    
    if(firstChild.isNull()) {
        // has no children, print out the text if any
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " has no children");
        //childInfo.addAttribute("text", webElement->toPlainText());
    }
    else {
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " traverse first child");
        traverseWebElement(&childInfo, parentPos, screenPos, &firstChild, webFrameId);
    }

    // check if this node has siblings and traverse them
    QWebElement sibling = webElement->nextSibling();
    if(sibling.isNull()) {
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " has no siblings");
    }
    else {
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " traverse sibling");
        traverseWebElement(parent, parentPos, screenPos, &sibling, webFrameId);
    }
}

/*!
  Parse attributes for QWebElement
*/
void WebKitTraverse::parseAttributes(QWebElement* webElement, TasObject* objInfo)
{
    //TasLogger::logger()->debug("WebKitTaverse::parseAttributes for tag " + webElement->localName());

    if(webElement->hasAttributes()) {
        foreach(QString attrib, webElement->attributeNames()) {
            //TasLogger::logger()->debug("  : " + attrib + "->" + webElement->attribute(attrib));
            objInfo->addAttribute(attrib, webElement->attribute(attrib));
        }
    }

}
#endif
