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
 


#include "testabilityutils.h"
#include "taslogger.h"
#include "testabilitysettings.h"
#include "tasqtcommandmodel.h"

/*!
  \class TestabilityUtils
  \brief TasServiveBase is an abstract base class for tasplugin services
    
*/

/*!
  Casting direclty from the id does not work so we need to look
  for the object with a matching id.   
*/
QWidget* TestabilityUtils::findWidget(const QString& id)
{
    TasLogger::logger()->debug("TestabilityUtils::findWidget id:" + id);
    QWidget* widget = NULL;
    QWidgetList widgetList = qApp->allWidgets();        
    if (!widgetList.empty()){        
        QWidgetList::iterator i;
        for (i = widgetList.begin(); i != widgetList.end(); ++i){        
            QWidget* current = *i;        
            QString currentId = TasCoreUtils::objectId(current);
            if(currentId == id){
                widget = current;
                break;
            }
        }    
    }    
    return widget;  
}

/*!
  Casting direclty from the id does not work so we need to look
  for the object with a matching id.
    
  Get the correct QGraphicsItem for the given object id.
  Looks for the item from graphicsviews and scenes     
*/
QGraphicsItem* TestabilityUtils::findGraphicsItem(const QString& id)
{
    QGraphicsItem* item = NULL;
    QWidgetList widgetList = qApp->topLevelWidgets();        
    if (!widgetList.empty()){
        QList<QWidget*>::iterator iter;        
        for (iter = widgetList.begin(); iter != widgetList.end(); iter++){
            QWidget *current = *iter;
            item = findFromObject(id, current);
            if(item){
                break;
            }
        }
    }
    else { 
	    qDebug("TasCommander::getGraphicsItem the list of top level widgets is empty!");
    }    
    return item;    
}

/*!
  Looks for a graphicsitem with thte given id from the children of the
  given object.
*/
QGraphicsItem* TestabilityUtils::findFromObject(const QString& id, QObject* object)
{
    QGraphicsItem* item = NULL;
    //check view first
    if (!object) return NULL;

    QGraphicsView* view = qobject_cast<QGraphicsView*>(object);
    if (view){                
        item = lookForMatch(view->items(), id);
        if(!item){    
            QGraphicsScene* scene = view->scene();
            if (scene) {
                item = lookForMatch(scene->items(), id);
            }
        }
    }
    //look from possible scenes
    QGraphicsScene* scene = qobject_cast<QGraphicsScene*>(object);
    if (!item && scene) {
        item = lookForMatch(scene->items(), id);  
    }
    //look from child objects if item not on top level
    if(!item){
        QList<QObject*> objList = object->children();
        QList<QObject*>::iterator iter;        
        for (iter = objList.begin(); iter != objList.end(); iter++){
            QObject* obj = *iter;
            if (obj->isWidgetType()){
                item = findFromObject(id, obj);
                if(item){
                    break;
                }
            }
        }
    }
    return item;
}

/*!
  Iterates a list of graphicsitems looking for a match for the given id.
  NULL is returned if a match is not found.
*/
QGraphicsItem* TestabilityUtils::lookForMatch(QList<QGraphicsItem*> itemList, const QString& targetId)
{
    QGraphicsItem* item = NULL;
    if (!itemList.isEmpty()){
        foreach(QGraphicsItem* child, itemList){
            if(verifyGraphicsItemMatch(targetId, child)){
                item = child;
                break;
            }            
        }
    }    
    return item;
}

/*!
  Verifys that a given GraphicsItem refences matches to the given id.
  If the item is a decendant of QObject (e.g. GraphicsWIdget) then 
  the verification is done by casting the item to object and then 
  making the comparison.
  Returns true if the given item matches. 
*/
bool TestabilityUtils::verifyGraphicsItemMatch(const QString& targetId, QGraphicsItem* source)
{    
    bool doesMatch = false;
    //first check simple match
    QString sourceId = TestabilityUtils::graphicsItemId(source);
    
    if (sourceId == targetId){
        doesMatch = true;
    }
    else{        
        //is object decendant
        QGraphicsObject* object = qgraphicsitem_cast<QGraphicsObject*>(source);               
        if (object) {            
            sourceId = TasCoreUtils::objectId(object);
            if (sourceId == targetId){
                doesMatch = true;                        
            }   
        }
    }    
    return doesMatch;
}

/*!        
    Gets the viewport widget for the item through the scene and 
    graphics view. Can be null if the viewport could not be determined. 
    Sets the point to be the screen coordinate at the center point of the
    QGraphicsitem.
 */
QWidget* TestabilityUtils::viewPortAndPosition(QGraphicsItem* graphicsItem, QPoint& point)
{
    QGraphicsView* view = getViewForItem(graphicsItem);
    QWidget* widget = NULL;
    if(view){
        widget = view->viewport();            
    }

    if (widget){                    
        QRectF sceneRect = graphicsItem->sceneBoundingRect();
        if(!view->viewportTransform().isIdentity()){
            QTransform transform = view->viewportTransform();
            sceneRect = transform.mapRect(sceneRect);
        }
        QRect viewPortRect = widget->rect();        
        QRect interSection = viewPortRect.intersected(sceneRect.toRect());
        point = widget->mapToGlobal(interSection.center());
    }
    return widget;
}   

QGraphicsView* TestabilityUtils::getViewForItem(QGraphicsItem* graphicsItem)
{
    if(!graphicsItem){
        return NULL;
    }
    QGraphicsView* match = NULL;

    QGraphicsScene* scene = graphicsItem->scene();
    if(scene){
        QList<QGraphicsView*> list = scene->views();    
        foreach(QGraphicsView* view, scene->views()){
            if(view->items().indexOf(graphicsItem) != -1){
                match = view;
                break;
            }
        }
    }
    return match;
}


/*! Return true if widgets belongs to custom traversed object, i.e. 
 * Will be traversed even if not visible */
bool TestabilityUtils::isCustomTraverse() {
    return getApplicationName() == "webwidgetrunner" ||
        getApplicationName() == "mappletrunner" ||
        getApplicationName() == "duiappletrunner";

}

/*!
   See if the GraphicsItems region intersects the viewports visible region.
 */
bool TestabilityUtils::isItemInView(QGraphicsView* view, QGraphicsItem* graphicsItem)
{
    if(view){
        QRectF sceneRect = graphicsItem->sceneBoundingRect();
        if(!view->viewportTransform().isIdentity()){
            QTransform transform = view->viewportTransform();
            sceneRect = transform.mapRect(sceneRect);
        }
        QRect viewPortRect = view->viewport()->rect();
        return viewPortRect.intersects(sceneRect.toRect());
    }
    else{
        return false;
    }
}



QWidget* TestabilityUtils::getApplicationWindow()
{
    //attemp to find a window type widget
    QWidget* target = qApp->activeWindow();
    if(!target || !target->isWindow() || target->graphicsProxyWidget()){
        TasLogger::logger()->debug("TestabilityUtils::getApplicationWindow no active window - look for suitable");
        //no active, take first from list and use it
        QWidgetList list = qApp->topLevelWidgets();
        QListIterator<QWidget*> iter(qApp->topLevelWidgets());
        while(iter.hasNext()){
            QWidget* w = iter.next();
            if((w->isVisible() || 
                (isCustomTraverse() && w->inherits("QGraphicsView")) )
               && w->isWindow() && w->graphicsProxyWidget() == 0){
                TasLogger::logger()->debug("TestabilityUtils::getApplicationWindow window found");
                target = w;
                break;
            }
        }                    
    }        
    return target;
}


bool TestabilityUtils::isBlackListed()
{
    QString filePath = qApp->applicationFilePath();
    QVariant value = TestabilitySettings::settings()->getValue(BLACK_LISTED);
    if(value.isValid() && value.canConvert<QString>()){
        QStringList blackList = value.toString().split(",");
        for (int i = 0; i < blackList.size(); i++){
            QString blackListed = blackList.at(i);
            if(filePath.contains(blackListed)){
                return true;
            }
        }
    }
    return false;
}

/*!        
    Returns the Proxy Widget if any parent widget has a proxy
 */
QGraphicsProxyWidget* TestabilityUtils::parentProxy(QWidget* widget) {
    if (!widget) {
        return 0;
    }

    QGraphicsProxyWidget* proxy = widget->graphicsProxyWidget();
    if (proxy) {
        return proxy;
    } else {
        return parentProxy(widget->parentWidget());
    }
}


QPoint TestabilityUtils::proxyCoordinates(QGraphicsItem* item, bool absolute) 
{
    QRectF sceneRect = item->sceneBoundingRect();
    QGraphicsView* view = TestabilityUtils::getViewForItem(item);
    if(!view->viewportTransform().isIdentity()){
        QTransform transform = view->viewportTransform();
        sceneRect = transform.mapRect(sceneRect);
    }
    if (absolute) {
        return view->viewport()->mapToGlobal(sceneRect.topLeft().toPoint());
    } else {
        return sceneRect.topLeft().toPoint();
    }
}

ItemLocationDetails TestabilityUtils::getItemLocationDetails(QGraphicsItem* graphicsItem, TasCommand* command)
{
    bool isVisible = false;
    QGraphicsView* view = getViewForItem(graphicsItem);
    ItemLocationDetails locationDetails;
    if(view){
        // If the window is embedded into a another app, wee need to figure out if the widget is visible
        QPoint windowSize(0,0);
        if (command && command->parameter("parent_size") != "") {
            QStringList sizes = command->parameter("parent_size").split(",");
            windowSize.setX(sizes[0].toInt());
            windowSize.setY(sizes[1].toInt());
        }
        isVisible = isItemInView(view, graphicsItem);

        if(isVisible){
            //add coordinates also            
            QRectF sceneRect = graphicsItem->sceneBoundingRect();
            if(!view->viewportTransform().isIdentity()){
                QTransform transform = view->viewportTransform();
                sceneRect = transform.mapRect(sceneRect);
            }
            QPoint point = sceneRect.topLeft().toPoint();
            QPoint screenPoint = view->viewport()->mapToGlobal(point);

            if (command && command->parameter("x_parent_absolute") != "" &&
                command->parameter("y_parent_absolute") != "") {
                QPoint p(command->parameter("x_parent_absolute").toInt(), 
                         command->parameter("y_parent_absolute").toInt());
                screenPoint += p;
                point += p;
              
            } 

            int height = sceneRect.height();
            int width = sceneRect.width();
            int xAbs = screenPoint.x();
            int yAbs = screenPoint.y();
            // Verify that the window is in application view
            if (windowSize != QPoint(0,0)) {
                // Windows with top left corner off screen are always off screen
                if (screenPoint.x() > windowSize.x() ||
                    screenPoint.y() > windowSize.y()) {
                    isVisible = false;
                } else {
                    // Window starts off-screen
                    if (xAbs < 0) {
                        width = xAbs+width;
                        xAbs = 0;
                    }
                    if (yAbs < 0) {
                        height = yAbs+height;
                        yAbs = 0;
                    }


                    // Window ends off-screen
                    if (xAbs+width > windowSize.x()) {
                        width = width - (xAbs+width - windowSize.x());
                    }
                    if (yAbs+height > windowSize.y()) {
                        height = height - (yAbs+height - windowSize.y());
                    }

                    if (width < 0 || height < 0) {
                        isVisible = false;
                    }
                    
                }

            }
            screenPoint.setX(xAbs);
            screenPoint.setY(yAbs);

            locationDetails.scenePoint = point;        
            locationDetails.screenPoint = screenPoint;
            locationDetails.width = width;
            locationDetails.height = height;
        }
    }
    locationDetails.visible = isVisible;
    return locationDetails;
}

QString TestabilityUtils::graphicsItemId(QGraphicsItem* graphicsItem)
{
    return QString::number((quintptr)graphicsItem);
}

QGraphicsWidget* TestabilityUtils::castToGraphicsWidget(QGraphicsItem* graphicsItem)
{
    if(graphicsItem && graphicsItem->isWidget()){
        return static_cast<QGraphicsWidget*>(graphicsItem);
    }
    return 0;
}

