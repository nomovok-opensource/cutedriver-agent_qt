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
#include <QListIterator>
#include <QtWebKit/QtWebKit>
#include <QGraphicsWebView>

#include "tascoreutils.h"
#include "tascommandparser.h"
#include "taslogger.h"
#include "webkitcommandservice.h"
#include "tassocket.h"

#include "actionhandler.h"
#include "gesturehandler.h"
#include "keyhandler.h"
#include "mousehandler.h"
#include "multitouchhandler.h"

#include "tasdeviceutils.h"

/*!
  \class WebkitCommandService
  \brief WebkitCommandService manages ui commands send to the app

*/    

WebkitCommandService::WebkitCommandService(QObject* parent)
    :QObject(parent), counter(0)
{}

WebkitCommandService::~WebkitCommandService()
{}

bool WebkitCommandService::executeService(TasCommandModel& model, TasResponse& response)
{    
    TasLogger::logger()->debug("WebkitCommandService::executeService " + model.service());
    //starting new id round
    counter = 0;
    if(model.service() == serviceName() ){
        foreach (TasTarget* target, model.targetList()) {
//            TasLogger::logger()->debug("WebkitCommandService::executeService " + target->type());
//            if (target->type() == TYPE_WEB) {

            	foreach (TasCommand* command, target->commandList()) {
//                    TasLogger::logger()->debug("WebkitCommandService::executeService command name:" + command->name());
                    bool ret = true;

                    if (command->name() == COMMAND_EXEC_SCROLL_QWEBFRAME){
                        ret = scrollQWebFrame(target, command);
                    }else
                    if (command->name() == COMMAND_EXEC_JS_ON_OBJ) {
                        ret = executeJavaScriptWebElement(target, command);
                    }else
                    if (command->name() == COMMAND_EXEC_JS_ON_QWEBFRAME){
                        ret = executeJavaScriptQWebFrame(target, command);
                    }

                    if(!ret) {
                        TasLogger::logger()->debug("WebkitCommandService::executeService failed, return true");
                        response.setErrorMessage(mErrorMessage);
                        return true;
                    }
                    else if(ret) {
                        TasLogger::logger()->debug("WebkitCommandService::executeService success, return true");
                        return true;
                    }
                }
  //          }
        }
        response.setErrorMessage("Webkit service target/command did not match any existing target or command");
        return false;
    }
    else{
        return false;
    }
}

bool WebkitCommandService::scrollQWebFrame(TasTarget* target, TasCommand* command)
{
    TasLogger::logger()->debug("WebkitCommandService::scrollQWebFrame  x,y(" +command->parameter("dx") + "," + command->parameter("dy") + ")");
    QString id = target->id();

    QList<QWebFrame*> mainFrameList;

    mainFrameList = traverseStart();

    foreach(QWebFrame* webFrame, mainFrameList)
    {
        bool ret;
        ret = traverserScrollToQWebFrame(webFrame, id, command->parameter("dx").toInt(), command->parameter("dy").toInt());
        if(ret) {
            return ret;
        }
    }

    mErrorMessage = "When scrolling QWebFrame: QWebFrame not found";
    return false;
}

bool WebkitCommandService::traverserScrollToQWebFrame(QWebFrame* webFrame, QString id, int dx, int dy)
{
    //TasLogger::logger()->debug("  id(" +  TasCoreUtils::objectId(webFrame) + "," + id + ")");
    if(TasCoreUtils::objectId(webFrame) == id){
        //TasLogger::logger()->debug("  found (" + id + ")");
        webFrame->scroll(dx,dy);
        return true;
    }
    foreach(QWebFrame* childFrame, webFrame->childFrames()) {
        bool ret = false;
        ret = traverserScrollToQWebFrame(childFrame,id,dx,dy);
        if(ret) {
            return ret;
        }
    }
    return false;
}


bool WebkitCommandService::executeJavaScriptWebElement(TasTarget* target, TasCommand* command)
{
    TasLogger::logger()->debug("WebkitCommandService::executeJavaScriptWebElement TasId("+ target->id() + ") JavaScript \"" + command->parameter("java_script")  + "\"");
    int index = command->parameter("index").toInt();
    index = (index == -1) ? 0 : index;

    QList<QWebFrame*> mainFrameList;

    mainFrameList = traverseStart();

//    TasLogger::logger()->debug("  mainFrameList size " + QString::number(mainFrameList.count()) );
    QString frameId = command->parameter("webframe_id");
    QString jScript = command->parameter("java_script");
    QString elementId = command->parameter("elementId");
    QString query = command->parameter("locator_query");

    foreach(QWebFrame* frame, mainFrameList){
        if(query.isEmpty()){
            if(executeJavascriptOnWebElement(frame, frameId, jScript, elementId)){
                return true;
            }
        }
        else{
            bool ret = false;
            ret = traverseJavaScriptToWebElement(frame, frameId, jScript, query, index, command);
            if(ret) return ret;
        }
    }

    //mErrorMessage = "When executing JavaScript to WebElement: QWebFrame not found";
    return false;
}

bool WebkitCommandService::executeJavaScriptQWebFrame(TasTarget* target, TasCommand* command)
{
    TasLogger::logger()->debug("WebkitCommandService::executeJavaScriptQWebFrame JavaScript \"" + command->parameter("java_script")  + "\"");
    QString id = target->id();

    QList<QWebFrame*> mainFrameList;

    mainFrameList = traverseStart();

    foreach(QWebFrame* frame, mainFrameList)
    {
        bool ret = false;
        ret = traverseJavaScriptToQWebFrame(frame,
                                            command->parameter("java_script"),
                                            id);
        if(ret) return ret;
    }

    mErrorMessage = "When executing JavaScript to QWebFrame: QWebFrame not found";
    return false;

}


bool WebkitCommandService::executeJavascriptOnWebElement(QWebFrame* webFrame, QString webFrameId, QString javaScript, QString elementId)
{    
    bool success = false;
    QWebFrame* targetFrame = 0;
    //this is the frame we want
    if( webFrameId == TasCoreUtils::objectId(webFrame)){
        targetFrame = webFrame;
    }
    //need to look for it from child frames
    else{
        targetFrame = lookForWebFrame(webFrame, webFrameId);
    }
    if(targetFrame){
        //frame found! lets look for the element
        QWebElement element = lookForWebElement(targetFrame->documentElement(), elementId, webFrameId);
        if(!element.isNull()){
            element.evaluateJavaScript(javaScript);
            success = true;   
        }
        else{
            mErrorMessage = "When executing JavaScript to WebElement: QWebElement not found";
        }
    }            
    else{
        mErrorMessage = "When executing JavaScript to WebElement: QWebFrame not found";        
    }
    return success;
}

QWebFrame* WebkitCommandService::lookForWebFrame(QWebFrame* webFrame, QString webFrameId)
{
    QWebFrame* targetFrame = 0;
    foreach(QWebFrame* childFrame, webFrame->childFrames()) {
        if(webFrameId == TasCoreUtils::objectId(childFrame)){
            targetFrame = childFrame;
            break;
        }
        else{
            targetFrame = lookForWebFrame(childFrame, webFrameId);
        }
        if(targetFrame){
            break;
        }
    }
    return targetFrame;
}

QWebElement WebkitCommandService::lookForWebElement(const QWebElement &parentElement, QString elementId, QString webFrameId)
{
    TasLogger::logger()->debug("WebkitCommandService::lookForWebElement elementid " + elementId);
    QWebElement match;
    QWebElement element = parentElement.firstChild();
    while (!element.isNull()) {        
        TasLogger::logger()->debug("WebkitCommandService::lookForWebElement candidate " + TasCoreUtils::pointerId(&element));
        QString candidateId = QString::number(qHash(element.toOuterXml() + webFrameId));
        if(elementId == candidateId){
            match = element;
        }
        else{
            match = lookForWebElement(element, elementId, webFrameId);
        }
        if(!match.isNull()){
            TasLogger::logger()->debug("WebkitCommandService::lookForWebElement match found " + elementId);
            break;
        }
        element = element.nextSibling();
    }
    return match;
}
                                                          
bool WebkitCommandService::traverseJavaScriptToWebElement(QWebFrame* webFrame,
                                                          QString webFrameId,
                                                          QString javaScript,
                                                          QString query,
                                                          int &index,
                                                          TasCommand* command,
                                                          int parentFrames)
{
    TasLogger::logger()->debug("WebkitCommandService::traverseJavaScriptToWebElement index(" + QString::number(index) +
                               ") webFrameId(" + webFrameId +
                               ") webFrame(" + TasCoreUtils::objectId(webFrame) + ")");
    QList<QWebElement> element_list;
    int count = 0;

    //find all elements, is matching webframe or if doing search to all webframes
    if(webFrameId == 0 || webFrameId == TasCoreUtils::objectId(webFrame))
    {
        TasLogger::logger()->debug("  ok to search elements " );

         //findAllElements
         //check do we have other limitations than tag type
         if(query.indexOf('[') > -1 )
         {

             // query string is like a[id='value'][text='literals']
             // split query to QHash:
             //     "a[id='value'"
             //     "[text='literals'"
             // - then find the attribute between [ and =
             // - then find value between ' and '
             QHash<QString, QString> attribHash;
             attribHash.insert("type", query.left(query.indexOf('[')));
             TasLogger::logger()->debug("    type(" + query.left(query.indexOf('[')) + ")");
             QStringList parameterList = query.split("]", QString::SkipEmptyParts);
             foreach(QString s,parameterList)
             {
                 QString attribute = s.mid(s.indexOf('[')+1, s.indexOf('=')-s.indexOf('[')-1);
                 QString value = s.mid(s.indexOf('\'')+1, s.lastIndexOf('\'')-s.indexOf('\'')-1);

                 if(attribute.size()>0){
                     TasLogger::logger()->debug("    attrib(" + attribute + ") value(" + value + ")");
                     attribHash.insert(attribute, value);
                 }
             }

             QPoint webViewPos(0,0);
             QPoint screenPos(0,0);

             element_list = traverseFrame(webFrame, webViewPos, screenPos,attribHash);

         } else {
             element_list = webFrame->findAllElements(query).toList();
         }

         count = element_list.count();
    }

    TasLogger::logger()->debug("WebkitCommandService::executeJavaScriptQWebFrame JavaScript \"" + javaScript  +
                               "\" query \"" + query +
                               "\" index(" + QString::number(index) +
                               ") count(" + QString::number(count) + ")");

    if(count > index)
    {
        QWebElement element = element_list.at(index);
        element.evaluateJavaScript(javaScript);
        return true;
    }
    else{
        // consume index if any matching objects were found
        index -= count;
        // find all direct children frames and traverse those too, return on first true
        foreach(QWebFrame* childFrame, webFrame->childFrames()) {
           bool ret = false;
           ret = traverseJavaScriptToWebElement(childFrame,
                                                webFrameId,
                                                javaScript,
                                                query,
                                                index,
                                                command,
                                                parentFrames+1);
           if(ret) {
               return ret;
           }
        }
        mErrorMessage = "When executing JavaScript to WebElement: QWebElement not found";
        return false;
    }
 }

bool WebkitCommandService::traverseJavaScriptToQWebFrame(QWebFrame* webFrame, QString javaScript, QString id)
{
    TasLogger::logger()->debug("WebkitCommandService::traverseJavaScriptToQWebFrame id " + id + "cast id: " + TasCoreUtils::objectId(webFrame) + ".");
    if(TasCoreUtils::objectId(webFrame) == id)
    {
//        TasLogger::logger()->debug("WebkitCommandService::traverseJavaScriptToQWebFrame found");
        webFrame->evaluateJavaScript(javaScript);
        return true;
    }
    else
    {
//        TasLogger::logger()->debug("WebkitCommandService::traverseJavaScriptToQWebFrame not found");
        // find all direct children frames and traverse those too
        foreach(QWebFrame* childFrame, webFrame->childFrames()) {
            bool ret = false;
            ret = traverseJavaScriptToQWebFrame(childFrame,javaScript,id);
            if(ret) {
                return ret;
            }
        }
    }
    mErrorMessage =  "When executing JavaScript to QWebFrame: QWebFrame not found";
    return false;
}

QList<QWebFrame*> WebkitCommandService::traverseObject(QObject* object)
{
    QList<QWebFrame*> list;

    //check decendants
    if(object->inherits("QWebView")){
        TasLogger::logger()->debug(" QWebView found " + QString(object->metaObject()->className()) );
        QWebView* web = qobject_cast<QWebView*>(object);
        if (web) {
            list.append(web->page()->mainFrame());
        }
    }
    else if(object->inherits("QGraphicsWebView")) {
        TasLogger::logger()->debug(" QGraphicsWebView found " + QString(object->metaObject()->className()) );
        QGraphicsWebView* web = qobject_cast<QGraphicsWebView*>(object);
        if (web) {
            list.append(web->page()->mainFrame());
        }
    }
    else // support for Symbian CWRT 9.2 and 10.1 - Fullscreen mode only
    if (object->inherits("WRT__WrtWebView")) {
        TasLogger::logger()->debug("WebKitCommandService::traverseObject WRT__WrtWebView");
        
        QGraphicsWebView* web = 0;    
        QMetaObject::invokeMethod(object, "view", Qt::DirectConnection, Q_RETURN_ARG(QGraphicsWebView*, web));
                
        if (web) {
            TasLogger::logger()->debug(" QGraphicsWebView found " + QString(object->metaObject()->className()) );            
            list.append(web->page()->mainFrame());
        }
    }

    //1. is graphicsview
    if(object->inherits("QGraphicsView")){
        list.append(traverseGraphicsViewItems(qobject_cast<QGraphicsView*>(object)));
    }
    //2. is GraphicsWidget
    QGraphicsWidget* graphicsWidget = qobject_cast<QGraphicsWidget*>(object);
    if(graphicsWidget){
        list.append(traverseGraphicsItemList(graphicsWidget));
    }
    //3. Widget children
    else{
        QObjectList children = object->children();
        if (!children.isEmpty()) {
            for (int i = 0; i < children.size(); ++i){
                QObject *obj = children.at(i);
                //TasLogger::logger()->debug(" "+ QString(obj->metaObject()->className()) );
                //only include widgets
                if (obj->isWidgetType() && obj->parent() == object){
                    QWidget *widget = qobject_cast<QWidget*>(obj);
                    // TODO This (and other similar hacks) needs to be moved to plugins once OSS changes are done
                    list.append(traverseObject(widget));
                }
            }
        }
    }
    return list;
}

QList<QWebFrame*> WebkitCommandService::traverseGraphicsItem(QGraphicsItem* graphicsItem)
{
    QList<QWebFrame*> list;
    if (graphicsItem->isWindow() || graphicsItem->isWidget()) {
        QObject * object = (QObject*)((QGraphicsWidget*)graphicsItem);
        list.append(traverseObject(object));
        // Traverse the actual widget under the proxy, if available
        QGraphicsProxyWidget* proxy = qobject_cast<QGraphicsProxyWidget*>(object);
        if (proxy) {
            list.append(traverseObject(proxy->widget()));
        }
    }
    else{
        list.append(traverseGraphicsItemList(graphicsItem));
    }
    return list;
}

QList<QWebFrame*> WebkitCommandService::traverseGraphicsItemList(QGraphicsItem* graphicsItem)
{
    QList<QWebFrame*> list;
    foreach (QGraphicsItem* item, graphicsItem->childItems()){
//        TasLogger::logger()->debug(" "+ QString(item->metaObject()->className()) );
        if(graphicsItem == item->parentItem()){
            // TODO This needs to be moved to plugins once OSS changes are done
            if(getApplicationName() == "webwidgetrunner" || item->isVisible() ||
               getApplicationName() == "duiappletrunner") {
               list.append(traverseGraphicsItem(item));
            }
        }
    }
    return list;
}

QList<QWebFrame*> WebkitCommandService::traverseGraphicsViewItems(QGraphicsView* view)
{
    QList<QWebFrame*> list;
    foreach(QGraphicsItem* item, view->items()){
//        TasLogger::logger()->debug("  "+ QString(item) );
        if(item->parentItem() == 0){
            // TODO This needs to be moved to plugins once OSS changes are done
            if(getApplicationName() == "webwidgetrunner" || item->isVisible() ||
               getApplicationName() == "duiappletrunner") {
                list.append(traverseGraphicsItem(item));
            }
        }
    }
    return list;
}

QList<QWebFrame*> WebkitCommandService::traverseStart()
{
    QList<QWebFrame*> list;

    foreach (QWidget *object, QApplication::allWidgets())
    {
//        TasLogger::logger()->debug(" "+ QString(object->metaObject()->className()) );
        list.append(traverseObject(object));
    }
    return list;
}


QList<QWebElement> WebkitCommandService::traverseFrame(QWebFrame* webFrame, const QPoint& parentPos, const QPoint& screenPos, QHash<QString, QString> attributeMatchHash)
{
    QList<QWebElement> list;
    if(webFrame) {
//        TasLogger::logger()->debug("WebkitCommandService::traverseFrame webFrame != null");


        QWebElement docElement = webFrame->documentElement();
        list.append(traverseWebElement(parentPos+webFrame->pos()-webFrame->scrollPosition(),
                                       screenPos+webFrame->pos()-webFrame->scrollPosition(),
                                       &docElement,
                                       attributeMatchHash));
   }
   return list;
}

/*!
  Traverse QWebElement
*/
QList<QWebElement> WebkitCommandService::traverseWebElement(QPoint parentPos,
                                                            QPoint screenPos,
                                                            QWebElement* webElement,
                                                            QHash<QString, QString> attributeMatchHash)
{
    QList<QWebElement> list;
    if(webElement == NULL || webElement->isNull()) {
        //TasLogger::logger()->debug("WebkitCommandService::traverseWebElement webElement is null");
        return list;
    }

    // traverse this element and all children

    //check that the parameters match for the web element
    QHashIterator<QString, QString> i(attributeMatchHash);
    QPoint childPos = QPoint(webElement->geometry().x(), webElement->geometry().y());
    counter++;
    bool okElement=true;
    while (i.hasNext() && okElement ) {
        i.next();
        if(webElement->attribute(i.key()) == i.value()){
            TasLogger::logger()->debug("  matched " + i.key() + " value \"" + i.value() + "\"" );
//        }else if(i.key() == "x" && i.value().toInt() == childPos.x()+parentPos.x()){
//            TasLogger::logger()->debug("  matched " + i.key());
//        }else if(i.key() == "y" && i.value().toInt() == childPos.y()+parentPos.y()){
//            TasLogger::logger()->debug("  matched " + i.key());
//        }else if(i.key() == "x_absolute" && i.value().toInt() == childPos.x()+screenPos.x()){
//            TasLogger::logger()->debug("  matched " + i.key());
//        }else if(i.key() == "y_absolute" && i.value().toInt() == childPos.y()+screenPos.y()){
//            TasLogger::logger()->debug("  matched " + i.key());
        }else if(i.key() == "width" && i.value().toInt() == webElement->geometry().width()){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "height" && i.value().toInt() == webElement->geometry().height()){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "objectType" && i.value() ==  "Web"){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "innerText" && i.value() ==  webElement->toPlainText()){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "name" && i.value() ==  webElement->localName().toLower()){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "id" && i.value().toInt() == counter  ){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "type" && i.value() == webElement->tagName().toLower()){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "visible" && ((i.value() == "true") == (webElement->styleProperty("visibility", QWebElement::ComputedStyle).toLower() == "visible"))){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "hasFocus" && ((i.value() == "true") == (webElement->hasFocus()))){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else if(i.key() == "elementText" && i.value() ==  parseElementText(webElement->toInnerXml())){
            TasLogger::logger()->debug("  matched " + i.key() + " " + i.value());
        }else {
            //did not match webelement, skip to next
            okElement = false;
//            TasLogger::logger()->debug("WebkitCommandService::traverseWebElement no element for key(" + i.key() + ") and value(" + i.value() + ")");
        }
    }
    if(okElement){
        TasLogger::logger()->debug("WebkitCommandService::traverseWebElement found element(" + webElement->tagName().toLower() + ")");
        list.append(*webElement);
    }
    //childInfo.addBooleanAttribute("visible", webElement->styleProperty("visibility", QWebElement::ComputedStyle).toLower() == "visible");


    // traverse first child
    QWebElement firstChild = webElement->firstChild();

    if(!firstChild.isNull()) {
        //TasLogger::logger()->debug("WebkitCommandService::traverseWebElement " + webElement->localName() + " traverse first child");
        list.append(traverseWebElement(parentPos, screenPos, &firstChild, attributeMatchHash));
    }

    // check if this node has siblings and traverse them
    QWebElement sibling = webElement->nextSibling();
    if(!sibling.isNull()) {
        //TasLogger::logger()->debug("WebkitCommandService::traverseWebElement " + webElement->localName() + " traverse sibling");
        list.append(traverseWebElement(parentPos, screenPos, &sibling, attributeMatchHash));
    }
    return list;
}

/*!
  Parse element text for QWebElement
*/
QString WebkitCommandService::parseElementText(QString innerXml)
{
    //TasLogger::logger()->debug("WebKitTaverse::parseElementText innerXml");
    QString ret;
    while(innerXml.size() > 0 && innerXml.contains('<')) {
        //add space if necessary
        if(ret.trimmed().size() > 0) {
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
        innerXml.remove(0, cut_gt+1);

        //continue removing until current and all possible similar child elements are removed
        int expectedEnds = 0;
        if(innerXml.indexOf("</" + elementName) >= 0){
            expectedEnds ++;
        }

        while(expectedEnds>0) {
            if(expectedEnds > 20)
            {
                return QString("failed to parse, probably inconsistent (x)html");
            }
            int nextElementTag = innerXml.indexOf("<" + elementName);
            int nextCloseElementTag = innerXml.indexOf("</" + elementName);

            //TasLogger::logger()->debug("  net:" + QString::number(nextElementTag) + "\n  ncet:" + QString::number(nextCloseElementTag));

            if(nextElementTag != -1 && nextElementTag < nextCloseElementTag) {
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
