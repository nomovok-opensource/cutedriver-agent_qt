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
#include "tastraverseutils.h"
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
  :QObject(parent), mTemporaryPointerToQGraphicsWebView(0)
{
    mTraverseUtils = new TasTraverseUtils();
}

/*!
    Destructor
*/
WebKitTraverse::~WebKitTraverse()
{
    delete mTraverseUtils;
}

void WebKitTraverse::beginTraverse(TasCommand* command)
{
    mTraverseUtils->createFilter(command);
}

void WebKitTraverse::endTraverse()
{
    mTraverseUtils->clearFilter();
}


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

    if (object->inherits("QWebView")){
        TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebView");
        QWebView* webView = qobject_cast<QWebView*>(object);
        if (webView){
            traverseQWebView(objectInfo, webView);
        }
    }

    if (object->inherits("QGraphicsWebView")){
        //TasLogger::logger()->debug("WebKitTaverse::traverseObject QGraphicsWebView");
        //store to class variable for to be used in treverserwebelement mapToScene
        mTemporaryPointerToQGraphicsWebView = qobject_cast<QGraphicsWebView*>(object);
        if (mTemporaryPointerToQGraphicsWebView){
            objectInfo->addAttribute("zoomScale", mTemporaryPointerToQGraphicsWebView->scale());
            traverseQGraphicsWebView(objectInfo, mTemporaryPointerToQGraphicsWebView, command);
        }
        mTemporaryPointerToQGraphicsWebView = 0;
    }



    if (object->inherits("DuiApplication")){
        //TasLogger::logger()->debug("WebKitTaverse::traverseObject DuiApplication.");
        
        QVariant angelPid = object->property("angelPid");
        
        if (angelPid.isValid()){
            int pid = angelPid.toInt();
            //TasLogger::logger()->debug("WebKitTaverse::traverseObject Actually an angel pid.");
                    TasObject& obj = objectInfo->addObject();
                    obj.setId(0);
                    obj.setType("TDriverRef");
                    obj.setName("TDriverRef");
                    //TasLogger::logger()->debug("WebKit::traverseObject referring to " + QString::number(pid));


                    addAttribute(obj, "uri", QString::number(pid));

            
            return;
        }



        QList<QObject*> list = object->findChildren<QObject*>();
        TasLogger::logger()->debug(">>>> Found : " + QString::number(list.size())  + " children");
        foreach (QObject* obj, list){
            const QMetaObject* mObj = obj->metaObject();
            TasLogger::logger()->debug(">>>> Found : " + QString(mObj->className()));


            if (QString(mObj->className()) == "WRT::Maemo::WebAppletRunner"){
                TasLogger::logger()->debug("Found communication server");
                QGraphicsWebView* view = 0;
                TasLogger::logger()->debug("Invoking method for communicationserver");
                
                QMetaObject::invokeMethod(obj, "view",
                                          Qt::DirectConnection,
                                          Q_RETURN_ARG(QGraphicsWebView*, view));
                if (view){
                    TasLogger::logger()->debug("Traversing webpage");
                    TasObject& obj = objectInfo->addObject();
                    QPoint p;
                    // Retrieve parent xy coords
                    TasLogger::logger()->debug("Trying to retrieve parent xy");
                    QString xpos = command->parameter("x_parent_absolute");
                    QString ypos = command->parameter("y_parent_absolute");
                    if (!xpos.isEmpty() && !ypos.isEmpty()){
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
    if (object->inherits("WRT__WrtWebView")){
        TasLogger::logger()->debug("WebKitTaverse::traverseObject WRT__WrtWebView");
        
        QGraphicsWebView* view = 0;
        QMetaObject::invokeMethod(object, "view",
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(QGraphicsWebView*, view));

        if (view){
            TasLogger::logger()->debug("Traversing webpage");
            TasObject& webPageObject = objectInfo->addObject();
            QPoint p;
            // Retrieve parent xy coords
            TasLogger::logger()->debug("Trying to retrieve parent xy");
            QString xpos = "0"; //command->parameter("x_parent_absolute");
            QString ypos = "0"; //command->parameter("y_parent_absolute");
            if (!xpos.isEmpty() && !ypos.isEmpty()){
                TasLogger::logger()->debug("XY: " + xpos + " " + ypos);

                p = QPoint(xpos.toInt(), ypos.toInt());
            }
            traverseQWebPage(webPageObject,view->page(), p, p);
        } else {
            TasLogger::logger()->debug("Page instance not found");
        }

    }

    if (object->inherits("GVA::WebChromeItem")){
//        TasLogger::logger()->debug(" WebKitTaverse::traverseObject found " + QString(object->metaObject()->className()) );

        QWebElement* element = 0;
        QMetaObject::invokeMethod(object, "element",
                                  Qt::AutoConnection,
                                  Q_RETURN_ARG(QWebElement*, element));
        if (element){
//          TasLogger::logger()->debug("Traversing webelement");
          TasObject& tas_object = objectInfo->addObject();

          QGraphicsWidget* item = qobject_cast<QGraphicsWidget*>(object);
          if (item){
              QPoint p(item->pos().x() - element->geometry().x(),item->pos().y() - element->geometry().y());
              QString tasId = TasCoreUtils::objectId(object);
              traverseWebElement(&tas_object,p,p,element, tasId);
          }
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
    objectInfo->setType("QGraphicsWebView");

    QWebPage* webPage = webView->page();

    if (webPage){

        //QPoint webViewPos = webView->mapTo(webView->window(), QPoint(0, 0));

        QPair<QPoint,QPoint>  coords = mTraverseUtils->addGraphicsItemCoordinates(objectInfo, webView, command);

        //TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebPage != null");
        TasObject& pageInfo = objectInfo->addObject();

        //pageInfo.addAttribute("objectType", TYPE_GRAPHICS_VIEW);
        addAttribute(pageInfo, "objectType", TYPE_GRAPHICS_VIEW);

        traverseQWebPage(pageInfo, webPage, coords.first, coords.second);
    }
    
//     QPoint screenPos = webView->mapToGlobal(QPoint(0, 0));
//     QGraphicsProxyWidget* proxy = TestabilityUtils::parentProxy(webView);
//     if (proxy){
//         // If the webview is inside a graphics proxy,
//         // Take the proxy widget global position
//         screenPos = TestabilityUtils::proxyCoordinates(proxy);
//         // Also move the webview pos to the proxy pos
//         webViewPos += TestabilityUtils::proxyCoordinates(proxy, false);
//     }


    
}

void WebKitTraverse::traverseQWebView(TasObject* objectInfo, QWebView* webView) 
{
//    TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebView != null");
    objectInfo->setType("QWebView");

    QWebPage* webPage = webView->page();

    if (webPage){

        QPoint webViewPos = webView->mapTo(webView->window(), QPoint(0, 0));
        QPoint screenPos = webView->mapToGlobal(QPoint(0, 0));

        QGraphicsProxyWidget* proxy = TestabilityUtils::parentProxy(webView);

        if (proxy){
            // If the webview is inside a graphics proxy,
            // Take the proxy widget global position
            screenPos = TestabilityUtils::proxyCoordinates(proxy);
            // Also move the webview pos to the proxy pos
            webViewPos += TestabilityUtils::proxyCoordinates(proxy, false);
        }

        //TasLogger::logger()->debug("WebKitTaverse::traverseObject QWebPage != null");
        TasObject& pageInfo = objectInfo->addObject();

        //pageInfo.addAttribute("objectType", TYPE_STANDARD_VIEW);
        addAttribute(pageInfo, "objectType", TYPE_STANDARD_VIEW);

        traverseQWebPage(pageInfo, webPage, webViewPos,screenPos);
    }

}

void WebKitTraverse::addAttribute( TasObject& object, const QString &name, QWebPage* webPage )
{

    if (!mTraverseUtils->includeAttribute(name)){
      return;
    }

    if (name == "isModified"){
      object.addBooleanAttribute(name, webPage->isModified());

    } else if (name == "totalBytes"){
      object.addAttribute(name, QString::number(webPage->totalBytes()));

    } else if (name == "receivedBytes"){
      object.addAttribute(name, QString::number(webPage->bytesReceived()));

    } else if (name == "selectedText"){
      object.addAttribute(name, webPage->selectedText());

    } else if (name == "viewPortWidth"){
      object.addAttribute(name, webPage->viewportSize().width());

    } else if (name == "viewPortHeight"){
      object.addAttribute(name, webPage->viewportSize().height());

    } else if (name == "isContentEditable"){
      object.addBooleanAttribute(name, webPage->isContentEditable());

    } else if (name == "width"){
      object.addAttribute(name, webPage->viewportSize().width());

    } else if (name == "height"){
      object.addAttribute(name, webPage->viewportSize().height());

    // unknown attribute
    } else {
      object.addAttribute( name, QString("UnknownAttributeName") );

    }

}

void WebKitTraverse::addAttribute( TasObject& object, const QString &name, const QPoint& point )
{

    if (!mTraverseUtils->includeAttribute(name)){
      return;
    }

    if (name == "x_absolute"){
      object.addAttribute("x_absolute", point.x());

    } else if (name == "y_absolute"){
      object.addAttribute("y_absolute", point.y());

    } else if (name == "x"){
      object.addAttribute("x", point.x());

    } else if (name == "y"){
      object.addAttribute("y", point.y());

    // unknown attribute
    } else {
      object.addAttribute( name, QString("UnknownAttributeName") );

    }

}

void WebKitTraverse::addAttribute( TasObject& object, const QString& name, const QString& string )
{

    if (mTraverseUtils->includeAttribute(name)){

      // add as is
      object.addAttribute(name, QString(string));

    }

}


void WebKitTraverse::addAttribute( TasObject& object, const QString &name, QWebElement* webElement )
{

    if (!mTraverseUtils->includeAttribute(name)){
      return;
    }

    if (name == "visibility" || name == "visible"){
      object.addBooleanAttribute(name, ( webElement->styleProperty("visibility", QWebElement::ComputedStyle).toLower() == "visible") && (webElement->styleProperty("display", QWebElement::ComputedStyle).toLower() != "none"));

    } else if (name == "style"){
      object.addAttribute( name, webElement->attribute("style"));

    } else if (name == "innerText"){
      object.addAttribute(name, webElement->toPlainText());

    } else if (name == "elementText"){
      object.addAttribute(name, parseElementText(webElement->toInnerXml()));

    } else if (name == "hasFocus"){
      object.addBooleanAttribute(name, webElement->hasFocus());

    } else if (name == "value"){
      object.addAttribute(name, webElement->evaluateJavaScript("this.value").toString() );

    } else if (name == "checked"){
      object.addAttribute(name, webElement->evaluateJavaScript("this.checked").toString());

    } else if (name == "selected"){
      object.addAttribute(name, webElement->evaluateJavaScript("this.selected").toString());

    } else if (name == "id"){
      object.addAttribute(name, webElement->evaluateJavaScript("this.id").toString());

    // unknown attribute
    } else {

      object.addAttribute( name, QString("UnknownAttributeName") );

    }

}

void WebKitTraverse::addAttribute( TasObject& object, const QString &name, int value )
{

    if (mTraverseUtils->includeAttribute(name)){
      // add as is
      object.addAttribute(name, value);
    }

}

void WebKitTraverse::addAttribute( TasObject& object, const QString &name, bool value )
{

    if (mTraverseUtils->includeAttribute(name)){
      // add as is
      object.addBooleanAttribute(name, value);
    }

}

void WebKitTraverse::addAttribute( TasObject& object, const QString &name, const char* const chars )
{

    if (mTraverseUtils->includeAttribute(name)){
      // add as is
      object.addAttribute(name, chars);
    }

}

void WebKitTraverse::addAttribute( TasObject& object, const QString &name, QWebFrame* webFrame )
{

    if (!mTraverseUtils->includeAttribute(name)){
      return;
    }

    if (name == "url"){
      object.addAttribute(name, webFrame->url().toString());

    } else if (name == "requestedUrl"){
      object.addAttribute(name, webFrame->requestedUrl().toString());

    } else if (name == "baseUrl"){
      object.addAttribute(name, webFrame->baseUrl().toString());

    } else if (name == "horizontalScrollBarHeight"){
      object.addAttribute(name, webFrame->scrollBarGeometry(Qt::Horizontal).height());

    } else if (name == "verticalScrollBarWidth"){
      object.addAttribute(name ,webFrame->scrollBarGeometry(Qt::Vertical).width());

    } else if (name == "title"){
      object.addAttribute(name, webFrame->title());

    // unknown attribute
    } else {
      object.addAttribute( name, QString("UnknownAttributeName") );

    }

}

void WebKitTraverse::traverseQWebPage(TasObject& pageInfo, QWebPage* webPage, const QPoint& webViewPos, const QPoint& screenPos) 
{

      pageInfo.setId(TasCoreUtils::objectId(webPage));

      pageInfo.setType("QWebPage");
      pageInfo.setName("QWebPage");


/*
      pageInfo.addAttribute("objectType", TYPE_QWEB);
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
*/

      addAttribute(pageInfo, "objectType", TYPE_QWEB);
      addAttribute(pageInfo, "isModified", webPage);
      addAttribute(pageInfo, "totalBytes", webPage);
      addAttribute(pageInfo, "receivedBytes", webPage);
      addAttribute(pageInfo, "selectedText", webPage);
      addAttribute(pageInfo, "viewPortWidth", webPage);
      addAttribute(pageInfo, "viewPortHeight", webPage);
      addAttribute(pageInfo, "isContentEditable", webPage);

      addAttribute(pageInfo, "x_absolute", screenPos);
      addAttribute(pageInfo, "y_absolute", screenPos);
      addAttribute(pageInfo, "x", webViewPos);
      addAttribute(pageInfo, "y", webViewPos);

      addAttribute(pageInfo, "width",  webPage);
      addAttribute(pageInfo, "height", webPage);

      mCropRect.setRect(webViewPos.x(), webViewPos.y(),webPage->viewportSize().width(), webPage->viewportSize().height());

      traverseFrame(webPage->mainFrame(), pageInfo, TasCoreUtils::objectId(webPage), webViewPos, screenPos);
}

void WebKitTraverse::traverseFrame(QWebFrame* webFrame, TasObject& parent, QString parentId, const QPoint& parentPos, const QPoint& screenPos)
{
    if (webFrame){

//        TasLogger::logger()->debug("WebKitTaverse::traverseFrame webFrame != null");
        //TasLogger::logger()->debug("WebKitTaverse::traverseObject " + webFrame->renderTreeDump());

        QString tasId = TasCoreUtils::objectId(webFrame);

        TasObject& frameInfo = parent.addObject();

        QString frameName = webFrame->frameName();

        frameInfo.setName(frameName);
        frameInfo.setId(tasId);
        frameInfo.setType("QWebFrame");
        frameInfo.setParentId(parentId);

        addAttribute(frameInfo, "objectType", QString(TYPE_WEB));

        addAttribute(frameInfo, "title", webFrame);
        addAttribute(frameInfo, "url", webFrame);
        addAttribute(frameInfo, "requestedUrl", webFrame);
        addAttribute(frameInfo, "baseUrl", webFrame);
        addAttribute(frameInfo, "horizontalScrollBarHeight", webFrame);
        addAttribute(frameInfo, "verticalScrollBarWidth", webFrame);

        int width = 0;
        int height = 0;

        int x = 0;
        int y = 0;

        //calculate only if needed
        if (mTraverseUtils->includeAttribute("x") || mTraverseUtils->includeAttribute("y") ||
            mTraverseUtils->includeAttribute("x_absolute") || mTraverseUtils->includeAttribute("y_absolute") ||
            mTraverseUtils->includeAttribute("width") || mTraverseUtils->includeAttribute("height") ){

          width  = webFrame->geometry().width();
          height = webFrame->geometry().height();

          x = parentPos.x() + webFrame->pos().x();
          y = parentPos.y() + webFrame->pos().y();

        }

        if (frameName.trimmed().startsWith("<!--framePath",Qt::CaseInsensitive)){
            
            QWebFrame* parentFrame = webFrame->parentFrame();

            QWebElement frameSet = parentFrame->findFirstElement("frameset");
            
            if (!frameSet.isNull()){

                QWebElementCollection frames = frameSet.findAll("frame");

                // Find frame index for this frame
                int i = frameName.trimmed().lastIndexOf("frame") + 5;

                if (i<=frameName.trimmed().length()){

                    QString frameNo = frameName.trimmed().at(i);

                    if (frameNo.toInt()<frames.count()){

                        QWebElement selFrame = frames[frameNo.toInt()];

                        //TasLogger::logger()->debug("visibility:" + selFrame.styleProperty("visibility", QWebElement::ComputedStyle).toLower());
                        //TasLogger::logger()->debug("display:" + selFrame.styleProperty("display", QWebElement::ComputedStyle).toLower());

                        addAttribute( frameInfo, "visibility", &selFrame );
                        addAttribute( frameInfo, "style", &selFrame );

                        if (mTemporaryPointerToQGraphicsWebView){

                            if (mTraverseUtils->includeAttribute("x") || mTraverseUtils->includeAttribute("y") ||
                                mTraverseUtils->includeAttribute("x_absolute") || mTraverseUtils->includeAttribute("y_absolute") ||
                                mTraverseUtils->includeAttribute("width") || mTraverseUtils->includeAttribute("height") ){

                              const QPointF elPos( selFrame.geometry().x() + parentPos.x(), selFrame.geometry().y() + parentPos.y());
                              const QPointF elPosRect( selFrame.geometry().width() + elPos.x(), selFrame.geometry().height() + elPos.y());

                              QPointF p = elPos;
                              QPointF pRect = elPosRect;

                              QPointF scenePos = mTemporaryPointerToQGraphicsWebView->mapToScene(p);
                              QPointF scenePosH = mTemporaryPointerToQGraphicsWebView->mapToScene(pRect);

                              QGraphicsScene* sc = mTemporaryPointerToQGraphicsWebView->scene();
                              QList<QGraphicsView*> gvList = sc->views();
                              QGraphicsView* gv = gvList[0];

                              QPointF gvPos = gv->mapFromScene(scenePos);
                              QPointF gvPosRect = gv->mapFromScene(scenePosH);
                              QPoint screenPosition = gv->mapToGlobal(gvPos.toPoint());
                              QPoint screenPositionRect = gv->mapToGlobal(gvPosRect.toPoint());

                              height = screenPositionRect.y() - screenPosition.y();
                              width  = screenPositionRect.x() - screenPosition.x();

                              x = screenPosition.x() - screenPos.x();
                              y = screenPosition.y() - screenPos.y();

                            }
                        }
                    }
                }
            }
        }
        else{

          addAttribute(frameInfo, "visibility", true);

        }

        addAttribute(frameInfo, "x", x);
        addAttribute(frameInfo, "y", y);
#ifdef Q_OS_SYMBIAN
        addAttribute(frameInfo, "x_absolute", x );
        addAttribute(frameInfo, "y_absolute", y );
#else
        addAttribute(frameInfo, "x_absolute", x + screenPos.x());
        addAttribute(frameInfo, "y_absolute", y + screenPos.y());
#endif
        addAttribute(frameInfo, "width", width);
        addAttribute(frameInfo, "height", height);

/*
        traverseWebElement(&frameInfo, parentPos + webFrame->pos() - webFrame->scrollPosition(), screenPos + webFrame->pos() - webFrame->scrollPosition(), &docElement, tasId);

        // find all direct children frames and traverse those too
        QList<QWebFrame*> frames = webFrame->childFrames();

        for(int i = 0; i < frames.size(); i++){
            traverseFrame(frames[i], frameInfo, tasId, parentPos + webFrame->pos() - webFrame->scrollPosition(), screenPos + webFrame->pos() - webFrame->scrollPosition() );
        }
*/

        QPoint _parentPos = parentPos + webFrame->pos() - webFrame->scrollPosition();
        //QPoint _screenPos = screenPos + webFrame->pos() - webFrame->scrollPosition();

        QWebElement docElement = webFrame->documentElement();

        traverseWebElement(&frameInfo, _parentPos, screenPos, &docElement, tasId);

        // find all direct children frames and traverse those too
        QList<QWebFrame*> frames = webFrame->childFrames();

        for(int i = 0; i < frames.size(); i++){
            traverseFrame(frames[i], frameInfo, tasId, _parentPos, screenPos );
        }
   }
}

/*!
  Traverse QWebElement
*/
void WebKitTraverse::traverseWebElement(TasObject* parent, QPoint parentPos, QPoint screenPos, QWebElement* webElement, const QString& webFrameId)
{
    if (webElement == NULL || webElement->isNull()){
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement webElement is null");
        return;
    }

    // traverse this element and all children
    //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement traverse this element:" + webElement->tagName());
    int x=0,y=0,x_absolute=0, y_absolute=0, width=0, height=0;

/* TUUKKA: taken out as blocks children from being traversed
    if (mTraverseUtils->includeAttribute("x") || mTraverseUtils->includeAttribute("y") || mTraverseUtils->includeAttribute("x_absolute") || mTraverseUtils->includeAttribute("y_absolute") || mTraverseUtils->includeAttribute("width") || mTraverseUtils->includeAttribute("height") ){
*/
      if (mTemporaryPointerToQGraphicsWebView){

        const QPointF elPos( webElement->geometry().x() + parentPos.x(), webElement->geometry().y() + parentPos.y());
        const QPointF elPosRect( webElement->geometry().width() + elPos.x(), webElement->geometry().height() + elPos.y());

        QPointF p = elPos;
        QPointF pRect = elPosRect;

        QPointF scenePos = mTemporaryPointerToQGraphicsWebView->mapToScene(p);
        QPointF scenePosH = mTemporaryPointerToQGraphicsWebView->mapToScene(pRect);

        QGraphicsScene* sc = mTemporaryPointerToQGraphicsWebView->scene();
        QList<QGraphicsView*> gvList = sc->views();
        QGraphicsView* gv = gvList[0];

        QPointF gvPos = gv->mapFromScene(scenePos);
        QPointF gvPosRect = gv->mapFromScene(scenePosH);

        QPoint screenPosition = gv->mapToGlobal(gvPos.toPoint());
        QPoint screenPositionRect = gv->mapToGlobal(gvPosRect.toPoint());

        height = screenPositionRect.y() - screenPosition.y();
        width = screenPositionRect.x() - screenPosition.x();

        // position need to be relative to parent
        // NOTE all of these needs to be handled in webkitcommandservice.cpp

        x = screenPosition.x() - screenPos.x(); // screenPosition
        y = screenPosition.y() - screenPos.y(); // screenPosition

#ifdef Q_OS_SYMBIAN
        x_absolute = screenPosition.x() - screenPos.x(); // screenPosition
        y_absolute = screenPosition.y() - screenPos.y(); // screenPosition
#else
        x_absolute = screenPosition.x();
        y_absolute = screenPosition.y();
#endif
      } else {

          QPoint childPos = QPoint( webElement->geometry().x(), webElement->geometry().y() );

          x = childPos.x() + parentPos.x();
          y = childPos.y() + parentPos.y();
          width = webElement->geometry().width();
          height = webElement->geometry().width();

#ifdef Q_OS_SYMBIAN
          x_absolute = childPos.x() + parentPos.x();
          y_absolute = childPos.y() + parentPos.y();
#else
          x_absolute = childPos.x() + screenPos.x();
          y_absolute = childPos.y() + screenPos.y();
#endif
      }
      //  }


    //if use_crop enabled, check if intercsected by view and if not do not traverse this item
    if(mTraverseUtils->useViewCrop() && !mCropRect.intersects(QRect(x,y,width,height)) || height == 0)
    {
        // TasLogger::logger()->debug("discarding element " + webElement->tagName());
//        TasLogger::logger()->debug("discarding element " + webElement->tagName() + "\n" +
//                                   "rect1 x(" + QString::number(mCropRect.x()) + ") y(" + QString::number(mCropRect.y()) +
//                                   ") w(" + QString::number(mCropRect.width()) + ") h(" + QString::number(mCropRect.height()) + ")\n" +

//                                   "rect2 x(" + QString::number(x) + ") y(" +QString::number(y) +
//                                   ") w(" + QString::number(width) + ") h(" + QString::number(height) + ")\n"
//                                   );
        //not adding current element, continue with siblings
    }
    else
    {
        TasObject& childInfo = parent->addObject();        

        uint elementId = qHash(webElement->toOuterXml() + webFrameId);

        childInfo.setId(QString::number(elementId));
        childInfo.setType(webElement->tagName().toLower().replace(QString(":"), QString("_")));
        childInfo.setName(webElement->localName().toLower());



        //childInfo.addAttribute("webFrame", webFrameId);
        addAttribute(childInfo, "webFrame", webFrameId);

        //rect
        addAttribute(childInfo, "x", x);
        addAttribute(childInfo, "y", y);

        addAttribute(childInfo, "x_absolute", x_absolute);
        addAttribute(childInfo, "y_absolute", y_absolute);

        addAttribute(childInfo, "width", width);
        addAttribute(childInfo, "height", height);



        addAttribute(childInfo, "objectType", TYPE_WEB);
        addAttribute(childInfo, "visible", webElement);
        addAttribute(childInfo, "hasFocus", webElement);

        if (webElement->hasAttributes()){
            parseAttributes(webElement, childInfo);
        }

        addAttribute(childInfo, "innerText", webElement);
        addAttribute(childInfo, "elementText", webElement);

        // traverse first child
        QWebElement firstChild = webElement->firstChild();

        if (firstChild.isNull()){
            // has no children, print out the text if any
            //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " has no children");
            //childInfo.addAttribute("text", webElement->toPlainText());
        } else {
            //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " traverse first child");
            traverseWebElement(&childInfo, parentPos, screenPos, &firstChild, webFrameId);
        }
    }

    // check if this node has siblings and traverse them
    QWebElement sibling = webElement->nextSibling();

    if (sibling.isNull()){
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " has no siblings");
    } else {
        //TasLogger::logger()->debug("WebKitTaverse::traverseWebElement " + webElement->localName() + " traverse sibling");
        traverseWebElement(parent, parentPos, screenPos, &sibling, webFrameId);
    }
}

void WebKitTraverse::parseAttributes(QWebElement* webElement, TasObject& objInfo)
{
    //TasLogger::logger()->debug("WebKitTaverse::parseAttributes for tag " + webElement->localName());

    if (webElement->hasAttributes()){
        foreach(QString attrib, webElement->attributeNames()){
            attrib = attrib.trimmed();
            if ( attrib != "id" && attrib != "width" && attrib != "height"){
                if ((attrib != "value" && attrib != "checked" ) || (webElement->localName().toLower() != "input") ){
                    if (mTraverseUtils->includeAttribute(attrib)){
                        objInfo.addAttribute(attrib, webElement->attribute(attrib));
                    }
                }
            }
        }
    }

/*
    if (webElement->localName().toLower() == "input" || webElement->localName().toLower() == "select"){
      addAttribute(objInfo, "value", webElement);
    }

    if (webElement->attribute("type") == "radio" || webElement->attribute("type") == "checkbox"){
      addAttribute(objInfo, "checked", webElement);
    }

    if ( webElement->attribute("type") == "option" || webElement->localName().toLower() == "option" ){
      addAttribute(objInfo, "selected", webElement);
    }
*/

    const QString localName = webElement->localName().toLower();
    const QString type = webElement->attribute("type");

    if (localName == "input" || localName == "select" || localName == "textarea"){
      addAttribute(objInfo, "value", webElement);
    }

    if (type == "radio" || type == "checkbox"){
      addAttribute(objInfo, "checked", webElement);
    }

    if (type == "option" || localName == "option" ){
      addAttribute(objInfo, "selected", webElement);
    }

    addAttribute(objInfo, "id", webElement);

}



/*!
  Parse element text for QWebElement
*/
QString WebKitTraverse::parseElementText(QString innerXml)
{
    //TasLogger::logger()->debug("WebKitTaverse::parseElementText innerXml");
    QString ret;
    while(innerXml.size() > 0 && innerXml.contains('<')){
        //add space if necessary
        if (ret.trimmed().size() > 0){
            ret = ret.trimmed() + " ";
        }

        // beginning, part before '<'
        ret += innerXml.left(innerXml.indexOf('<')).trimmed();
        //TasLogger::logger()->debug("  ret: " + ret);
        innerXml.remove(0, innerXml.indexOf('<')+1);
        //TasLogger::logger()->debug("  innerXml: " + innerXml);

        //element name
        int cut_space = innerXml.indexOf(' ');
        int cut_gt    = innerXml.indexOf('>');
        int cut       = cut_space > cut_gt ? cut_gt : cut_space;

        //TasLogger::logger()->debug("  s:" + QString::number(cut_space) + "\n  g:" + QString::number(cut_gt) + "\n  c:" + QString::number(cut));

        QString elementName;
        elementName += innerXml.left(cut).trimmed();
        //TasLogger::logger()->debug("  ele: " + elementName);
        innerXml.remove(0, cut_gt + 1);

        //continue removing until current and all possible similar child elements are removed
        int expectedEnds = 0;

        if (innerXml.indexOf("</" + elementName) >= 0){
            expectedEnds ++;
        }

        while(expectedEnds>0){
            if (expectedEnds > 20)
            {
                return QString("failed to parse, probably inconsistent (x)html");
            }
            int nextElementTag = innerXml.indexOf("<" + elementName);
            int nextCloseElementTag = innerXml.indexOf("</" + elementName);

            //TasLogger::logger()->debug("  net:" + QString::number(nextElementTag) + "\n  ncet:" + QString::number(nextCloseElementTag));

            if (nextElementTag != -1 && nextElementTag < nextCloseElementTag){
                //TasLogger::logger()->debug("  start tag found");
                innerXml.remove(0, nextElementTag + 1);
                expectedEnds++;
            } else {
                //TasLogger::logger()->debug("  end tag found");
                innerXml.remove(0, nextCloseElementTag + 1);
                expectedEnds--;
            }
            innerXml.remove(0, innerXml.indexOf('>')+1);

        }
    }
    ret += innerXml.trimmed();
    //TasLogger::logger()->debug("WebKitTaverse::parseElementText innerXml ended");
    return ret.trimmed();
}

#endif
