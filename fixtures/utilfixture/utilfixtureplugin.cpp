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
#include <QtGui>
#include <QApplication>
#include <QtPlugin>
#include <QDebug>
#include <QHash>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QtTest/qtestspontaneevent.h>
#include <QXmlStreamReader>
#include <taslogger.h>
#include <QFile>
#include <QDomDocument>
#include <QTest>
#include <QList>
//#include "ChromelessWidgetFrame.h"
#include "utilfixtureplugin.h"
#include "testabilityutils.h"
#include "taslogger.h"
#include "AsyncInstall.h"
//#include "testhelpers.h"
#include "WidgetManager.h"
#include "WebAppRegistry.h"

#ifdef QTWRT_USE_USIF
#include <usif/scr/scr.h>
#include <usif/scr/screntries.h>
#include <usif/scr/appreginfo.h>
#include "w3csettingskeys.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#endif

//#include "SuperWidget.h"
//#include "configw3xmlparser.h"
QString uninstallStatus="";
QString installStatus="";
Q_EXPORT_PLUGIN2(utilfixtureplugin, UtilFixturePlugin)

/*!
  \class UtilFixturePlugin
  \brief TasDuiTraverse traverse DUI components for adding custom details to them
        
  Using standard qt objects it is not always possible to get specific details from the 
  components. This TasHelperInterface implementation will allow us to add details from 
  the wanted dui components that are not accesible through the property or other
  generic way. 
*/

/*!
  Constructor
*/
    UtilFixturePlugin::UtilFixturePlugin(QObject* parent)
        :QObject(parent)
{}

/*!
  Destructor
*/
UtilFixturePlugin::~UtilFixturePlugin()
{}

/*!
  Implementation for traverse so always true.
*/
bool UtilFixturePlugin::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    // place your own code below
    bool result = true;

    if(actionName.toLower() == "showmessage"){
        QMessageBox msgBox;        
        msgBox.setText(parameters.value("text"));
        msgBox.exec();
    }
    // set the stdOut if you wish to pass information back to MATTI
    else if(actionName.toLower() == "fail"){
        stdOut = "The execution failed. Parameters were {";
        result =  false;
    }
    else if(actionName.toLower() == "contextmenu"){
        TasLogger::logger()->debug("UtilFixturePlugin::execute contextMenu");
        QObject* target = 0;
        QPoint globalPoint(-1, -1);
        QPoint localPoint(-1, -1);
        if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget");
            QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
            if (widget) {
                localPoint = widget->rect().center();
                globalPoint = widget->mapToGlobal(localPoint);
                target = qobject_cast<QObject*>(widget);
            }
        }
        else if(parameters.value(OBJECT_TYPE) == GRAPHICS_ITEM_TYPE ){
            TasLogger::logger()->debug("UtilFixturePlugin::execute graphicsitem");
            QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);
            if(item) {
                QWidget* widget = TestabilityUtils::viewPortAndPosition(item, globalPoint);
                localPoint = widget->mapFromGlobal(globalPoint);
                target = qobject_cast<QObject*>(widget);
            }
        }

        if(localPoint.x() > -1 && localPoint.y() > -1) {
            TasLogger::logger()->debug("UtilFixturePlugin::execute local point x:" + QString().setNum(localPoint.x()) + " y:" + QString().setNum(localPoint.y()) + " global point x:" + QString().setNum(globalPoint.x()) + " y:" + QString().setNum(globalPoint.y()));
            //QEvent event = QContextMenuEvent(QContextMenuEvent::Other, point );
            //QEvent event = QContextMenuEvent(QContextMenuEvent::Mouse, localPoint, globalPoint );
            QContextMenuEvent e(QContextMenuEvent::Mouse, localPoint, globalPoint, 0);
            QSpontaneKeyEvent::setSpontaneous(&e);
            qApp->notify(target, &e);
        }
    }
    else if(actionName.toLower() == "ensurevisible"){
        TasLogger::logger()->debug("UtilFixturePlugin::execute ensureVisible");
        QWidget* widget = 0;
        QGraphicsItem* item = 0;

        if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
            TasLogger::logger()->debug("UtilFixturePlugin::execute ensureVisible widget type");
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget");
            widget = reinterpret_cast<QWidget*>(objectInstance);
        }
        
        if (widget) {
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget != null");
            item = reinterpret_cast<QGraphicsItem*>(widget);
        } else {
            TasLogger::logger()->debug("UtilFixturePlugin::execute widget == null");
            item = reinterpret_cast<QGraphicsItem*>(objectInstance);
        }

        if (item) {
            TasLogger::logger()->debug("UtilFixturePlugin::execute item != null");
            QGraphicsView* view = TestabilityUtils::getViewForItem(item);
            if (view) {
                view->ensureVisible(item);
            }
            else {
                TasLogger::logger()->debug("UtilFixturePlugin::execute view == null");
            }
        }
    }
    else if(actionName.toLower() == "setfocus"){
		if(parameters.value(OBJECT_TYPE) == WIDGET_TYPE ){
			QWidget* widget = reinterpret_cast<QWidget*>(objectInstance);
			if(widget){
				widget->setFocus(Qt::MouseFocusReason);
			}
		}
		else if(parameters.value(OBJECT_TYPE) == GRAPHICS_ITEM_TYPE ){
			QGraphicsItem* item = reinterpret_cast<QGraphicsItem*>(objectInstance);  
			if(item){
				item->setFocus(Qt::MouseFocusReason); 
			}
		}
	}
	  //isWidgetRunning
    else if(actionName.toLower() == "iswidgetrunning"){
    QString key;
    QString value;
        QHash<QString, QString>::const_iterator i = parameters.constBegin();
        while (i != parameters.constEnd()) {

            key=i.key();
            //stdOut.append("The key is" + key);
            value=i.value();
            ++i;
            
            if (key.toLower().compare("bundleid")==0){
             break;
            }
        }
      TBool myResult = EFalse;
      myResult = isWidgetRunning(value);
      if (myResult){
        stdOut.append("True");
      }else{
        stdOut.append("False");
      }
    }
    else if(actionName.toLower()=="getinstalledwidgetbundleids"){
      stdOut = getBundleIds();
    }
    else if(actionName.toLower()=="getinstalledwidgetattributes"){
        stdOut = getInstalledWidgetAttributes();
    }
    else if(actionName.toLower()=="getbundleidfromwidgetid"){
        QString key;
        QString value;
        QHash<QString, QString>::const_iterator i = parameters.constBegin();
          while (i != parameters.constEnd()) {
    
              key=i.key().toLower();
              //stdOut.append("The key is" + key);
              value=i.value();
              ++i;
              
              if (key.toLower().compare("widgetid")==0){
               break;
              }
          }
        stdOut = getBundleIDfromWidgetID(value);
    }else if(actionName.toLower()=="getbundlenamefromwidgetid"){
        QString key;
        QString value;
        QHash<QString, QString>::const_iterator i = parameters.constBegin();
          while (i != parameters.constEnd()) {
    
              key=i.key().toLower();
              //stdOut.append("The key is" + key);
              value=i.value();
              ++i;
              
              if (key.toLower().compare("widgetid")==0){
               break;
              }
          }
        stdOut = getBundleNamefromWidgetID(value);       
        
    }
    else if(actionName.toLower() =="installwidget"){
        TasLogger::logger()->debug("actionName is installWidget");
        QString key;
        QString path;
        QString value;
        QString qSilent;
        QString qUpdate;
        bool silent = EFalse;
        bool update = EFalse;
        QObject *parent = this;
        QString rootDir;
        
        
        QHash<QString, QString>::const_iterator i = parameters.constBegin();
          while (i != parameters.constEnd()) {
    
              key=i.key().toLower();
              //stdOut.append("The key is" + key);
              value=i.value();
              ++i;
              
              if (key.toLower().compare("path")==0){
                  path = value;
              }
              else if (key.toLower().compare("silent")==0){
                  qSilent = value;
                  if (qSilent.toLower() == "true"){
                       silent = true;  
                  }     
              }
              else if (key.toLower().compare("update")==0){
                  qUpdate = value;
                  if (qUpdate.toLower() == "true"){
                      update = true;
                  }
              }
              else if (key.toLower().compare("rootdir")==0){
                  rootDir = value;
              }
          }
       //now attempt to install the widget and report our outcome.
       if (widgetInstall(path, silent, update, parent, rootDir)){
           stdOut = path + "Install Success";
       }else{
           stdOut = path + "Install Failed";
       }
    }
    else if(actionName.toLower() == "uninstallwidget"){
     QString key;
     QString value;
     QString qSilent;
     bool silent = EFalse;
     //QObject *parent = this;
     QString widgetId;
     
     
     QHash<QString, QString>::const_iterator i = parameters.constBegin();
       while (i != parameters.constEnd()) {
 
           key=i.key().toLower();
           //stdOut.append("The key is" + key);
           value=i.value();
           ++i;
           
           if (key.toLower().compare("silent")==0){
               qSilent = value;
               if (qSilent.toLower() == "true"){
                    silent = true;  
               }
           }

           else if (key.toLower().compare("widgetid")==0){
               widgetId = value;
           }
       }
        //now attempt to remove the widget and report our outcome.
       
        if (widgetUninstall(widgetId, silent)){
            stdOut = "uninstall Success";
        }else{
            stdOut = "Uninstall Failed";
        }
    }
    else if(actionName.toLower() == "getwidgetuninstallstatus"){
        stdOut = uninstallStatus;
        uninstallStatus ="";
    }
    else if(actionName.toLower() == "getwidgetinstallstatus"){
        stdOut = installStatus;
        installStatus = "";
    }
    else if(actionName.toLower() == "getwidgetuidfrombundleid"){
        QString key;
        QString value;
        QString param;
        
        QHash<QString, QString>::const_iterator i = parameters.constBegin();
        while (i != parameters.constEnd()) {
    
          key=i.key().toLower();
          //stdOut.append("The key is" + key);
          value=i.value();
          ++i;
          
          if (key.toLower().compare("bundleid")==0){
              param = value;
              break;
          }
        }
        stdOut = getUidFromBundleID(value);
    }
    else{
        stdOut = "You didn't call a valid fixture function name. Function names are NOT case sensitive";
        result =  true;
    }
//    QHash<QString, QString>::const_iterator i = parameters.constBegin();
//    while (i != parameters.constEnd()) {
//        stdOut.append("(");
//        stdOut.append(i.key());
//        stdOut.append("=>");
//        stdOut.append(i.value());
//        stdOut.append(")");
//        ++i;
//        
//        stdOut.append("}");
//        // set the return value as boolean
//    }
    return result;
}

bool UtilFixturePlugin::isWidgetRunning(QString appId)
{
    TInt error = KErrNone;
    TBool isWidgetRunning = EFalse;
    TPtrC widgetName(static_cast<const TUint16*>(appId.utf16()));
    TasLogger::logger()->enableLogger();
    
    // query registry for UID for widget
#ifdef QTWRT_USE_USIF
    RSecureWidgetRegistryClientSession registryClient;   
    TUid uiUid(KNullUid);
    //get the widgetUID from the app ID.
    QString widgetUID = getBundleIDfromWidgetID(appId);
    TRAP(error, isWidgetRunning=isWidgetRunningL(widgetUID,uiUid));
    if (!error==0){
        return EFalse;
    }
    return isWidgetRunning;
      
#else
    RWidgetRegistryClientSession registryClient;
#endif
    error = registryClient.Connect();
    if (error != KErrNone){
      TasLogger::logger()->debug("utilfixtureplugin::isWidgetRunning could not connect to widget registry");
      return EFalse;
    }
    TUid widgetUid(KNullUid);
#ifdef QTWRT_USE_USIF
    RSecureWidgetInfoArray widgets;
#else
    RWidgetInfoArray widgets;
#endif
    
    TRAP(error, registryClient.InstalledWidgetsL(widgets));
    if (!error) {
        for (TInt i(0); i < widgets.Count(); ++i) {
            
#ifdef QTWRT_USE_USIF
            CSecureWidgetInfo* info = widgets[i];
#else   
            CWidgetInfo* info = widgets[i];
#endif
    
            TBuf16<128> bundleId;
            registryClient.GetWidgetBundleId(info->iUid, bundleId);
            if (bundleId.CompareF(widgetName) == 0) {
                isWidgetRunning = registryClient.IsWidgetRunning(info->iUid);
                if (isWidgetRunning)
                    break;
            }
            if(i==widgets.Count()){
              TasLogger::logger()->debug("utilfixtureplugin::isWidgetRunning the widget registry thinks " +appId+ " is not installed");
            }
        }
    }
    else{
      TasLogger::logger()->debug("utilfixtureplugin::isWidgetRunning the widget registry thinks there are no widgets installed");
      isWidgetRunning = ETrue;  
    }
    registryClient.Disconnect();
    return isWidgetRunning;
}


QString UtilFixturePlugin::getUidFromBundleID(QString myBundleID)
{
    //TBool isWidgetRunning = EFalse;
    //TPtrC widgetName(static_cast<const TUint16*>(myBundleID.utf16()));
    TasLogger::logger()->enableLogger();
    
    // query registry for UID for widget
#ifdef QTWRT_USE_USIF
    //identifier is synonymous with my UID.
    TInt identifier = WebAppRegistry::instance()->appIdTonativeId(myBundleID);
    TUid identUid;
    identUid.Uid(identifier); // 0x36E0E89C
    if (identifier == 0){
        return "utilfixtureplugin::getUIDfromWidgetID, 0 is" \
        " returned from appIdTonativeId("+myBundleID+")";
    }
    QString qidentifier;
    qidentifier=qidentifier.setNum(identifier);
    return qidentifier;
#else
    TInt error = KErrNone;
    QString returnValue ="";
    RWidgetRegistryClientSession registryClient;
    error = registryClient.Connect();
    if (error != KErrNone){
      return "utilfixtureplugin::isWidgetRunning could not connect to widget registry";
    }
    TUid widgetUid(KNullUid);
    
    RWidgetInfoArray widgets;
    TRAP(error, registryClient.InstalledWidgetsL(widgets));
    if (!error) {
        for (TInt i(0); i < widgets.Count(); ++i) {
            CWidgetInfo* info = widgets[i];
    
            // Note that bundleId is pushed onto the cleanup stack
            // and needs a PopAndDestroy() for cleanup later
            TBuf16<128> bundleId;
            TBuf16<128> bundleName;
            TBuf16<128> widgetPath;
            TInt uid;
            //TPt16 ptr(bundleId.Des());
            //registryClient.GetWidgetBundleId(info->iUid, ptr);
            registryClient.GetWidgetBundleId(info->iUid, bundleId);
            uid = registryClient.GetWidgetUidL(bundleId);
            registryClient.GetWidgetBundleName(info->iUid,bundleName);
            registryClient.GetWidgetPath(info->iUid, widgetPath);
            QString qString((QChar*)bundleId.Ptr(),bundleId.Length());
            QString qStringName((QChar*)bundleName.Ptr(),bundleName.Length());
            QString qStringPath((QChar*)widgetPath.Ptr(),widgetPath.Length());
            QString qUid = QString::number(uid);
            
            if (qString.compare(myBundleID) == 0){
             returnValue.append(qUid);
            }
        }
    }
    else{
      return "utilfixtureplugin::getUidFromBundleID the widget registry thinks there is no widget that matches bundleId" + myBundleID;
    }
    registryClient.Disconnect();
    return returnValue;
#endif
}

/*I don't think this function is needed. It seems to be doing 
 * what getInstalledWidgetAttribute is doing.*/
QString UtilFixturePlugin::getBundleIds()
{
    TInt error = KErrNone;
    //TBool isWidgetRunning = EFalse;
    //TPtrC widgetName(static_cast<const TUint16*>(appId.utf16()));
    TasLogger::logger()->enableLogger();
    QString returnValue ="";
    
    // query registry for UID for widget
#ifdef QTWRT_USE_USIF
    RSecureWidgetRegistryClientSession registryClient;
#else
    RWidgetRegistryClientSession registryClient;
#endif
    error = registryClient.Connect();
    if (error != KErrNone){
      return "utilfixtureplugin::isWidgetRunning could not connect to widget registry";
    }
    TUid widgetUid(KNullUid);
#ifdef QTWRT_USE_USIF
    RSecureWidgetInfoArray widgets;
#else
    RWidgetInfoArray widgets;
#endif
    TRAP(error, registryClient.InstalledWidgetsL(widgets));
    if (!error) {
        for (TInt i(0); i < widgets.Count(); ++i) {
#ifdef QTWRT_USE_USIF
            CSecureWidgetInfo* info = widgets[i];
#else   
            CWidgetInfo* info = widgets[i];
#endif
    
            // Note that bundleId is pushed onto the cleanup stack
            // and needs a PopAndDestroy() for cleanup later
            TBuf16<128> bundleId;
            TBuf16<128> bundleName;
            TBuf16<128> widgetPath;
            //TPt16 ptr(bundleId.Des());
            //registryClient.GetWidgetBundleId(info->iUid, ptr);
            registryClient.GetWidgetBundleId(info->iUid, bundleId);
            registryClient.GetWidgetBundleName(info->iUid,bundleName);
            registryClient.GetWidgetPath(info->iUid, widgetPath);
            QString qString((QChar*)bundleId.Ptr(),bundleId.Length());
            QString qStringName((QChar*)bundleName.Ptr(),bundleName.Length());
            QString qStringPath((QChar*)widgetPath.Ptr(),widgetPath.Length());
            
            returnValue.append(qString);
            returnValue.append(":");
            returnValue.append(qStringName);
            returnValue.append(":");
            returnValue.append(qStringPath);
            returnValue.append(",");
        }
    }
    else{
      return "utilfixtureplugin::getBundleIds the widget registry thinks there are no widgets installed";
    }
    registryClient.Disconnect();
    return returnValue;
}

/* This function returns comma delimited attributes for all installed widgets:
 * However it gets it from the config.xml file, it should instead get it
 * from the database.
 * 
 * for example:
 * bundleID = 7b3f59f8149cae750803716804d0b65119cfc201
 * bundleName = accuweather-w3c
 * widgetPath = c:\Private\200267C0\widgets_21D_4C7\1d4dde37132b2deca25746eee90003d505a93f24\config.xml
 * widgetID = com.nokia.widget.accuweather
 * uid = 12345678  (this is needed to pass to WidgetInstallerApp for uninstall)
 * */
QString UtilFixturePlugin::getInstalledWidgetAttributes()
{
    //ConfigXmlParser xmlparser("/tmp/config.xml");
    TInt error = KErrNone;
    //TBool isWidgetRunning = EFalse;
    //TPtrC widgetName(static_cast<const TUint16*>(appId.utf16()));
    TasLogger::logger()->enableLogger();
    QString returnValue ="";
    
    // query registry for UID for widget
#ifdef QTWRT_USE_USIF
    WebAppRegistry::instance();  //causes the db to open.
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");
    if (db.isOpen()) {
        // Get the id of the row with the passed appId
        QSqlQuery query(db);
        query.prepare("SELECT * FROM webapp_registry");
        //query.bindValue(":appId", QVariant(appId));
        query.exec();
        TInt i =0;
        TBool firstRow=ETrue;
        while (query.next()) {
            TInt id = query.value(0).toInt();
            QString qid;
            qid = qid.setNum(id);      
            
            /* 333.600  [Qt Message] Row  0 appId= "90d252178ae56282bf0281032c708980670fbc94" 
             * 333.600  [Qt Message] Row  0 appTitle= "Contacts_Automated" 
             * 333.600  [Qt Message] Row  0 appPath= "C:\Private\200267C0\widgets_21D_4C7\90d252178ae56282bf0281032c708980670fbc94\" 
             * 333.600  [Qt Message] Row  0 appType= "w3c-partial-v1" 
             */
            QString appIdRes    = query.value(1).toString();
            QString appTitleRes = query.value(2).toString();
            QString appPath     = query.value(3).toString();
            QString appType     = query.value(6).toString();
            QString widgetId    = WebAppRegistry::instance()->uniqueIdToWidgetId(appIdRes);
            QString widgetUid   = getUidFromBundleID(appIdRes);
            qDebug() << "Row " << i << "widgetId=" << widgetId;
            qDebug() << "Row " << i << "appId=" << appIdRes;
            qDebug() << "Row " << i << "appTitle=" << appTitleRes;
            qDebug() << "Row " << i << "appPath=" << appPath;
            qDebug() << "Row " << i << "appType=" << appType;
            qDebug() << "Row " << i << "appUid=" << widgetUid;
            i=i+1;
            
            if (!firstRow)
                returnValue.append(", "); 
            
            returnValue.append("bundleID = " +appIdRes+ " : bundleName = "+ appTitleRes+ " : widgetPath = " + appPath+" : widgetId = " + widgetId + " : uid = "+ widgetUid);
            firstRow=EFalse;
        }
        return returnValue;
    }
    return "UtilFixturePlugin::getInstalledWidgetAttributes -> The WebAppRegistry db was not opened!!";
    
#else
    RWidgetRegistryClientSession registryClient;
    error = registryClient.Connect();
    if (error != KErrNone){
      return "utilfixtureplugin::isWidgetRunning could not connect to widget registry";
    }
    TUid widgetUid(KNullUid);
    RWidgetInfoArray widgets;
    TRAP(error, registryClient.InstalledWidgetsL(widgets));
    if (!error) {
        if (widgets.Count()==0){
        return "The registry reports that there are 0 widgets installed!";
        }
        for (TInt i(0); i < widgets.Count(); ++i) {
            CWidgetInfo* info = widgets[i];
            
            TBuf16<128> bundleId;
            TBuf16<128> bundleName;
            TBuf16<128> widgetPath;
            TInt uid;
            //TPt16 ptr(bundleId.Des());
            //registryClient.GetWidgetBundleId(info->iUid, ptr);
            registryClient.GetWidgetBundleId(info->iUid, bundleId);
            uid = registryClient.GetWidgetUidL(bundleId);
            registryClient.GetWidgetBundleName(info->iUid,bundleName);
            registryClient.GetWidgetPath(info->iUid, widgetPath);
            widgetPath.Append(_L("config.xml"));
            QString qBundleID((QChar*)bundleId.Ptr(),bundleId.Length());
            QString qBundleName((QChar*)bundleName.Ptr(),bundleName.Length());
            QString qWidgetPath((QChar*)widgetPath.Ptr(),widgetPath.Length());
            QString qUid = QString::number(uid);
            //WidgetType widgetType = SuperWidget::getWidgetType(qWidgetPath,"application/widget");   
 
            QDomDocument *doc = new QDomDocument();
             QFile file( qWidgetPath);
             QString id ="";
             if( !file.open( QIODevice::ReadOnly) )
               //return "Cannot find the file";
                 continue;
             
             if( !doc->setContent( &file ))
             {
               file.close();
               return "failed to set the doc content type";
             }
             file.close();

             QDomElement root = doc->documentElement();
             QString tagName = root.tagName();
             if( tagName != "widget" )
               return "root tagname is not widget. Make sure fixture is opening config.xml!";
             id = root.attribute( "id", "the id attribute was not set" );
             //QDomNode n = root.firstChild();
              //while( !n.isNull() )
              //{
              //  QDomElement e = n.toElement();
              //  if( !e.isNull() )
              // {
              //    QString eTag = e.tagName();
              //    if( eTag == "widget" )
              //    {
              //      id = e.attribute( "id", "the id attribute was not set" );
              //      //QMessageBox::information( 0, "Contact", c.name + "\n" + c.phone + "\n" + c.eMail );
              //    }
              //  }
              
              //  n = n.nextSibling();
              //}
     
              returnValue.append("bundleID = " +qBundleID+ " : bundleName = "+ qBundleName+ " : widgetPath = " + qWidgetPath +" : widgetId = " + id + " : uid = "+ qUid);
              //returnValue.append(":");
              //returnValue.append(qBundleName);
              //returnValue.append(":");
              //returnValue.append(qWidgetPath);
              //returnValue.append(":");
              //returnValue.append(id);
              
              if (i < widgets.Count())
                  returnValue.append(", ");
          }
      }
      else{
        return "utilfixtureplugin::getWidgetIdFromURL the widget registry thinks there are no widgets installed";
      }
      registryClient.Disconnect();
      return returnValue;         
#endif

}


QString UtilFixturePlugin::getBundleIDfromWidgetID(QString appId)
{
    
    //TBool isWidgetRunning = EFalse;
    //TPtrC widgetName(static_cast<const TUint16*>(appId.utf16()));
    TasLogger::logger()->enableLogger();    
    // query registry for UID for widget
#ifdef QTWRT_USE_USIF
    //next line for dumping the widget registry.
    //WebAppRegistry::instance()->printAllRows();
    
    QString widgetUID;
    QString qwidgetName;
    //widgetUID is MY bundleID. Example: 10d51eadcbce4ed3480d80fbdfc9b2a106d3d620
    widgetUID = WebAppRegistry::instance()->widgetIdToUniqueId(appId);
    if (widgetUID.compare("")==0){
        return "UtilFixturePlugin::getBundleIDfromWidgetID -> " \
                "WebAppRegisty says "+appId+" is not installed.";
    }
    qwidgetName = getWidgetNameFromWidgetUniqueID(widgetUID);
    return widgetUID;
#else
    QString returnValue ="";
    TInt error = KErrNone;
    RWidgetRegistryClientSession registryClient;
    error = registryClient.Connect();
    if (error != KErrNone){
      return "utilfixtureplugin::getBundleIDfromWidgetID could not connect to widget registry";
    }
    TUid widgetUid(KNullUid);
    
    RWidgetInfoArray widgets;
    
    TRAP(error, registryClient.InstalledWidgetsL(widgets));
    if (!error) {
        for (TInt i(0); i < widgets.Count(); ++i) {
            CWidgetInfo* info = widgets[i];
            
            // Note that bundleId is pushed onto the cleanup stack
            // and needs a PopAndDestroy() for cleanup later
            TBuf16<128> bundleId;
            TBuf16<128> bundleUID;
            TBuf16<128> bundleName;
            TBuf16<128> widgetPath;
            //TPt16 ptr(bundleId.Des());
            //registryClient.GetWidgetBundleId(info->iUid, ptr);
            registryClient.GetWidgetBundleId(info->iUid, bundleId);
            registryClient.GetWidgetBundleName(info->iUid,bundleName);
            registryClient.GetWidgetPath(info->iUid, widgetPath);
            widgetPath.Append(_L("config.xml"));
            
            QString qBundleID((QChar*)bundleId.Ptr(),bundleId.Length());
            QString qBundleName((QChar*)bundleName.Ptr(),bundleName.Length());
            QString qWidgetPath((QChar*)widgetPath.Ptr(),widgetPath.Length());
            TasLogger::logger()->debug("UtilFixturePlugin::getBundleIDfromWidgetID the current widgetPath is: " + qWidgetPath);  
            
            QDomDocument *doc = new QDomDocument();
            QFile file( qWidgetPath);
            QString id ="";
            if( !file.open( QIODevice::ReadOnly) )
              //return "Cannot find the file";
              continue;
            if( !doc->setContent( &file ))
            {
              file.close();
              return "failed to set the doc content type";
            }
            file.close();

            QDomElement root = doc->documentElement();
            QString tagName = root.tagName();
            if( tagName != "widget" )
              return "root tagname is not widget. Make sure fixture is opening config.xml!";
            id = root.attribute( "id", "the id attribute was not set" );

            //now check to see if the widgetID relates to a bundleID.
            if (id.toLower() == appId.toLower()){
                returnValue.append(qBundleID);
                break;
            }
        }
        if (returnValue.length()==0) {
            returnValue.append("UtilFixturePlugin::getBundleIdFromWidgetId, Could not find a Widget with widgetID="+appId);
        }
    }
    else{
      return "utilfixtureplugin::getBundleIDfromWidgetID the widget registry thinks there are no widgets installed";
    }
    registryClient.Disconnect();
    return returnValue;
#endif
 
}

QString UtilFixturePlugin::getBundleNamefromWidgetID(QString appId)
{
    //TBool isWidgetRunning = EFalse;
    //TPtrC widgetName(static_cast<const TUint16*>(appId.utf16()));
    TasLogger::logger()->enableLogger();

    // query registry for UID for widget
#ifdef QTWRT_USE_USIF
    //next line for dumping the widget registry.
    //WebAppRegistry::instance()->printAllRows();
    
    QString widgetUID;
    QString qwidgetName;
    //widgetUID is MY bundleID. Example: 10d51eadcbce4ed3480d80fbdfc9b2a106d3d620
    widgetUID = WebAppRegistry::instance()->widgetIdToUniqueId(appId);
    if (widgetUID.compare("")==0){
        return "UtilFixturePlugin::getBundleNamefromWidgetID -> " \
                "WebAppRegisty says "+appId+" is not installed.";
    }
    qwidgetName = getWidgetNameFromWidgetUniqueID(widgetUID);
    if (qwidgetName.compare("")==0){
        return "UtilFixturePlugin::getBundleNamefromWidgetID -> " \
                "call to getWidgetNameFromWidgetUniqueID returned an empty string!";
    }
    return qwidgetName;  
    
#else
    TInt error = KErrNone;
    QString returnValue ="";
    RWidgetRegistryClientSession registryClient;
        error = registryClient.Connect();
    if (error != KErrNone){
      return "utilfixtureplugin::getBundleNameFromWidgetID could not connect to widget registry";
    }
    TUid widgetUid(KNullUid);
    RWidgetInfoArray widgets;
    TRAP(error, registryClient.InstalledWidgetsL(widgets));
    if (!error) {
        for (TInt i(0); i < widgets.Count(); ++i) {
            CWidgetInfo* info = widgets[i];
    
            // Note that bundleId is pushed onto the cleanup stack
            // and needs a PopAndDestroy() for cleanup later
            TBuf16<128> bundleId;
            TBuf16<128> bundleUID;
            TBuf16<128> bundleName;
            TBuf16<128> widgetPath;
            //TPt16 ptr(bundleId.Des());
            //registryClient.GetWidgetBundleId(info->iUid, ptr);
            registryClient.GetWidgetBundleId(info->iUid, bundleId);
            registryClient.GetWidgetBundleName(info->iUid,bundleName);
            registryClient.GetWidgetPath(info->iUid, widgetPath);
            widgetPath.Append(_L("config.xml"));
            
            QString qBundleID((QChar*)bundleId.Ptr(),bundleId.Length());
            QString qBundleName((QChar*)bundleName.Ptr(),bundleName.Length());
            QString qWidgetPath((QChar*)widgetPath.Ptr(),widgetPath.Length());
            TasLogger::logger()->debug("UtilFixturePlugin::getBundleIDfromWidgetID the current widgetPath is: " + qWidgetPath);  
            
            QDomDocument *doc = new QDomDocument();
            QFile file( qWidgetPath);
            QString id ="";
            if( !file.open( QIODevice::ReadOnly) )
              //return "Cannot find the file";
              continue;
            if( !doc->setContent( &file ))
            {
              file.close();
              return "failed to set the doc content type";
            }
            file.close();

            QDomElement root = doc->documentElement();
            QString tagName = root.tagName();
            if( tagName != "widget" )
              return "root tagname is not widget. Make sure fixture is opening config.xml!";
            id = root.attribute( "id", "the id attribute was not set" );

            //now check to see if the widgetID relates to a bundleID.
            if (id.toLower() == appId.toLower()){
                returnValue.append(qBundleName);
                break;
            }
        }
        if (returnValue.length()==0){
            returnValue.append("Could not find a Widget with widgetID="+appId);
        }
    }
    else{
      return "utilfixtureplugin::getBundleNamefromWidgetID the widget registry thinks there are no widgets installed";
    }
    registryClient.Disconnect();
    return returnValue;
#endif
}


bool UtilFixturePlugin::widgetInstall(const QString &path, bool silent, bool update, QObject *parent, const QString &rootDir){
    bool retValue = EFalse;
    installStatus = "";
    //trying to do a non-silent install crashes
    if (!silent){
        installStatus = "You can only perform silent install from this fixture";
        return EFalse;
    }
    
    //shush compiler
    QObject *myparent = parent;
    
    //we need to get an instance of QApplication or else silent install will crash!
    //int argc=1; 
    //char *argv[1];
    //char a[1][1];
    //char  (*argv)[1];
    //argv = a;

    //char **argv=NULL;
    //*argv[0]='a';
    //QApplication app(argc, qApp->argv());
    //QWidget w;
    //qApp->setActiveWindow(qApp->activeWindow());
    
    //QList<QAction*> softkeys;
   // QAction *keyActionLSK = new QAction(QString(), qApp->activeWindow());
    //keyActionLSK->setSoftKeyRole(QAction::PositiveSoftKey);
    //QAction *keyActionRSK = new QAction(QString(), qApp->activeWindow());
   // keyActionRSK->setSoftKeyRole(QAction::NegativeSoftKey);
    //softkeys.append(keyActionLSK);
    //softkeys.append(keyActionRSK);
    //qApp->activeWindow()->addActions(softkeys);

    //qApp->activeWindow()->showMaximized();
    //qApp->exec();
    TasLogger::logger()->debug("Got inside UtilFixturePlugin::widgetInstall");
    QString widgetId;
    
    TasLogger::logger()->debug("called enum WidgetInstallError");
    enum WidgetInstallError result(WidgetInstallFailed);
    
    
    TasLogger::logger()->debug("Get a WidgetManager instance");
    //using pointer here instead of "WidgetManager widgetMgr(NULL,true);" 
    //because later when we plan to implement signals we'll need this as pointer type.
    
    // old parameter   widgetFrame.parentWidget();
    WidgetManager *widgetMgr = new WidgetManager(0);
    
    if(!widgetMgr){
        TasLogger::logger()->debug("WidgetManager instance is NULL!!");
        return EFalse;
    }
    
    TasLogger::logger()->debug("Getting ready to set content type to application/widget");
    widgetMgr->setContentType("application/widget");
    
    TasLogger::logger()->debug("Commence the install on WidgetManager instance");
    result = widgetMgr->install(path, widgetId, silent, update, rootDir);
    
    if (result == WidgetInstallSuccess){
        retValue = ETrue;
    }

    switch(result){
        case WidgetInstallSuccess:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetInstallSuccess");
            installStatus = "WidgetInstallSuccess";
            retValue = ETrue;
            break;        
        case WidgetValidSignature:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetValidSignature");
            installStatus = "WidgetValidSignature";
            break;         
        case WidgetUnZipBundleFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetUnZipBundleFailed");
            installStatus = "WidgetUnZipBundleFailed";
            break;           
        case WidgetFindSignatureFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetFindSignatureFailed");
            installStatus = "WidgetFindSignatureFailed";
            break;
        case WidgetSignatureParsingFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetSignatureParsingFailed");
            installStatus = "WidgetSignatureParsingFailed";
            break;   
        case WidgetSignatureOrSignedInfoMissing:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetSignatureOrSignedInfoMissing");
            installStatus = "WidgetSignatureOrSignedInfoMissing";
            break;
        case WidgetSignatureRefExistFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetSignatureRefExistFailed");
            installStatus = "WidgetSignatureRefExistFailed";
            break;       
        case WidgetSignatureRefValidationFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetSignatureRefValidationFailed");
            installStatus = "WidgetSignatureRefValidationFailed";
            break;               
        case WidgetCertValidationFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetCertValidationFailed");
            installStatus = "WidgetCertValidationFailed";
            break;
        case WidgetSignatureValidationFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetSignatureValidationFailed");
            installStatus = "WidgetSignatureValidationFailed";
            break;           
        case WidgetParseManifestFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetParseManifestFailed");
            installStatus = "WidgetParseManifestFailed";
            break;           
        case WidgetRegistrationFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetRegistrationFailed");
            installStatus = "WidgetRegistrationFailed";
            break;          
        case WidgetReplaceFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetReplaceFailed");
            installStatus = "WidgetReplaceFailed";
            break;     
        case WidgetRmDirFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetRmDirFailed");
            installStatus = "WidgetRmDirFailed";
            break; 
        case WidgetCapabilityNotAllowed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetCapabilityNotAllowed");
            installStatus = "WidgetCapabilityNotAllowed";
            break;
        case WidgetPlatformSpecificInstallFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetPlatformSpecificInstallFailed");
            installStatus = "WidgetPlatformSpecificInstallFailed";
            break;
        case WidgetCorrupted:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetCorrupted");
            installStatus = "WidgetCorrupted";
            break;
        case WidgetSharedLibraryNotSigned:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetSharedLibraryNotSigned");
            installStatus = "WidgetSharedLibraryNotSigned";
            break;
        case WidgetDriveLetterValidationFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetDriveLetterValidationFailed");
            installStatus = "WidgetDriveLetterValidationFailed";
            break;
        case WidgetTypeValidationFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetTypeValidationFailed");
            installStatus = "WidgetTypeValidationFailed";
            break;
        case WidgetSystemError:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetSystemError");
            installStatus = "WidgetSystemError";
            break;
        case WidgetInstallPermissionFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetInstallPermissionFailed");
            installStatus = "WidgetInstallPermissionFailed";
            break;
        case WidgetUpdateFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetUpdateFailed");
            installStatus = "WidgetUpdateFailed";
            break;
        case WidgetUpdateVersionCheckFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetUpdateVersionCheckFailed");
            installStatus = "WidgetUpdateVersionCheckFailed";
            break;
        case WidgetUserConfirmFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetUserConfirmFailed");
            installStatus = "WidgetUserConfirmFailed";
            break;
        case WidgetInsufficientDiskSpace:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetInsufficientDiskSpace");
            installStatus = "WidgetInsufficientDiskSpace";
            break;
        case WidgetInstallFailed:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetInstallFailed");
            installStatus = "WidgetInstallFailed";
            break;
        case WidgetStartFileNotFound:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetStartFileNotFound");
            installStatus = "WidgetStartFileNotFound";
            break;
        case WidgetIdInvalid:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall WidgetIdInvalid");
            installStatus = "WidgetIdInvalid";
            break;
        default:
            TasLogger::logger()->debug("UtilFixturePlugin::widgetInstall fell through to default in switch statement");
            installStatus = "Widget Install Failed for an unknown reason!";
            break;
    }

    /*************************************************************************************
     * TODO: Here we wish to implement the signals so we can listen for them later.
     * Notice that the m_installer will need to be widgetMgr instead.*/
    //QWidget *myWidget = new QWidget(0,Qt::SubWindow);  //this crashes the device.
    //QObject *myParent = qApp->parent();
    //is path valid
    
    //WidgetManager *m_installer = new WidgetManager(myWidget);
    //WidgetManager *m_installer = new WidgetManager(aWidget);
    //QSignalSpy spy_installer(m_installer, SIGNAL(installationSucceed()));
    //QVERIFY(spy_installer.isValid());
    //QVERIFY(spy_installer.count() == 0);
    //m_installer->asyncInstall(path, silent, rootDir, update);

    //AsyncInstall *as = new AsyncInstall(path, silent, update, NULL, rootDir);
    //as->run();
    //QVERIFY(WaitForSignal(spy_installer, 1));
    //qDebug() << "**Verifying installer count: " << spy_installer.count();

    //QVERIFY(WaitForSignal(spy_installer, 1));
    //QList<QVariant> arguments2 = spy_installer.takeFirst();
    //QVERIFY(!arguments2.isEmpty());
   // qDebug() << "**loadFinished: " << arguments2.at(0).toBool();
    //if (arguments2.at(0).toBool() == true ){
    //  retValue = ETrue;
    //}
    //delete myParent;
    /**************************************************************************************/
    delete widgetMgr;
    TasLogger::logger()->debug("Now gracefully leaving UtilFixturePlugin::widgetInstall");
    return retValue;
}

bool UtilFixturePlugin::widgetUninstall(QString &uid, bool silent){
    TasLogger::logger()->debug("Got inside UtilFixturePlugin::widgetUninstall");
    bool retValue = EFalse;
    uninstallStatus = "";
    if (!silent){
        uninstallStatus = "This fixture cannot handle non-silent uninstall.";
        return retValue;
    }
    TasLogger::logger()->debug("UtilFixturePlugin::widgetUninstall Get instance of WidgetManager");
    WidgetManager widgetMgr(NULL,false);
    WidgetUninstallError wue;
    QString bundleId;
    
    bundleId = getBundleIDfromWidgetID(uid);
    TasLogger::logger()->debug("UtilFixturePlugin::widgetUninstall got bundleID: "+ bundleId+ " from WidgetId: "+uid);
    
    TasLogger::logger()->debug("UtilFixturePlugin::widgetUninstall calling uninstall now!!");
    wue = widgetMgr.uninstall(bundleId, silent);
    switch (wue)
    {
        case WidgetUninstallSuccess:
            TasLogger::logger()->debug("WidgetUninstallSuccess");
            uninstallStatus = "WidgetUninstallSuccess";
            retValue = ETrue;
            break;
        
        case WidgetUninstallFailed:
            uninstallStatus = "WidgetUninstallFailed";
            TasLogger::logger()->debug("WidgetUninstallFailed");
            break;
        
        case WidgetUninstallPermissionFailed:
            uninstallStatus = "WidgetUninstallPermissionFailed";
            TasLogger::logger()->debug("WidgetUninstallPermissionFailed");
            break;
        
        case WidgetUninstallCancelled:
            uninstallStatus = "WidgetUninstallCancelled";
            TasLogger::logger()->debug("WidgetUninstallCancelled");
            break;
            
        case WidgetUninstallSuccessButIncomplete:
            uninstallStatus = "WidgetUninstallSuccessButIncomplete";
            TasLogger::logger()->debug("WidgetUninstallSuccessButIncomplete");
            break;
        default :
            uninstallStatus = "WidgetUninstall Failed for some unknown reason";
            TasLogger::logger()->debug("WidgetUninstall Failed for some other reason");
            break;
    }

    TasLogger::logger()->debug("UtilFixturePlugin::widgetUninstall exiting the function gracefully");
    return retValue;
}


bool UtilFixturePlugin::WaitForSignal(QSignalSpy & spy_signal , int expected)
{
    int ccount = spy_signal.count();
    qDebug() << "Count: " << ccount << "Expected: " << expected;
    if (ccount >= expected ) {
        qDebug() << "Count already as expected (" << expected << ") or greater";
        return true ;
    }
    for (int iter = 0; (spy_signal.count() != ccount + 1) && iter < 10; ++iter)
        QTest::qWait(16000);
    qDebug() << "Count Return: " << spy_signal.count();
    return (ccount < spy_signal.count()? true : false);
}

QString UtilFixturePlugin::getWidgetNameFromWidgetUniqueID(QString appId){
#ifdef QTWRT_USE_USIF
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        // Get the id of the row with the passed appId
        QSqlQuery query(db);
        query.prepare("SELECT * FROM webapp_registry WHERE appId = :appId");
        query.bindValue(":appId", QVariant(appId));
        query.exec();
        TInt i =0;
        if (query.next()) {
            TInt id               = query.value(0).toInt();
            QString qid;
            qid = qid.setNum(id);
            
            QString appIdRes      = query.value(1).toString();
            QString appTitleRes   = query.value(2).toString();
            QString appPath       = query.value(3).toString();
            QString appType       = query.value(6).toString();
            qDebug() << "Row " << i << "appId=" << appIdRes;
            qDebug() << "Row " << i << "appTitle=" << appTitleRes;
            qDebug() << "Row " << i << "appPath=" << appPath;
            qDebug() << "Row " << i << "appType=" << appType;
            i=i+1;
            return appTitleRes;
        }
    }
    return "";
#else
    return "UtilFixturePlugin::getWidgetNameFromWidgetUniqueID -> Not implemented for this platform";
#endif
}

HBufC* qt_QString2HBufCNewL(const QString& aString)
{
    HBufC *buffer;
#ifdef QT_NO_UNICODE
    TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString.toLocal8Bit().constData()));
    buffer = HBufC8::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#else
    TPtrC16 ptr(reinterpret_cast<const TUint16*>(aString.utf16()));
    buffer = HBufC16::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#endif
    return buffer;
}

TBool UtilFixturePlugin::isWidgetRunningL(QString& uniqueID, TUid& uiUid) {
#ifdef QTWRT_USE_USIF
        
    TBool isWidgetRunning = false;
    HBufC *globalId = qt_QString2HBufCNewL(uniqueID);
    CleanupStack::PushL(globalId);
    HBufC *appIDKey = qt_QString2HBufCNewL(SCR_PROP_APPID);
    CleanupStack::PushL(appIDKey);
    HBufC *isActiveKey = qt_QString2HBufCNewL(SCR_PROP_ISACTIVE);
    CleanupStack::PushL(isActiveKey);
    HBufC *isPresentKey = qt_QString2HBufCNewL(SCR_PROP_ISPRESENT);
    CleanupStack::PushL(isPresentKey);
    HBufC *procIDKey = qt_QString2HBufCNewL(SCR_PROP_PROCUID);
    CleanupStack::PushL(procIDKey);
    
    Usif::RSoftwareComponentRegistry m_SCRClient;
    User::LeaveIfError(m_SCRClient.Connect());

    RArray<Usif::TComponentId> aComponentIdList;
    
    // Create & set filter
    Usif::CComponentFilter* myFilter = Usif::CComponentFilter::NewL();
    myFilter->SetSoftwareTypeL(Usif::KSoftwareTypeWidget);
    myFilter->AddPropertyL(*appIDKey, *globalId);  //appID is unique ID;
     
    m_SCRClient.GetComponentIdsL(aComponentIdList, myFilter);
    
    for(int i = 0; i < aComponentIdList.Count(); i++) {
        Usif::TComponentId id = (Usif::TComponentId) aComponentIdList[i];
        Usif::CComponentEntry* aEntry = Usif::CComponentEntry::NewL();
        
        if (m_SCRClient.GetComponentL(id, *aEntry)) {

            // Get PROCID
            Usif::CIntPropertyEntry* procIDPropEntry
                       = (Usif::CIntPropertyEntry*) m_SCRClient.GetComponentPropertyL(id, *procIDKey);
            uiUid.iUid = procIDPropEntry->IntValue();

            // Get IsPresent
            Usif::CIntPropertyEntry* isPresentPropEntry
                             =  (Usif::CIntPropertyEntry*) m_SCRClient.GetComponentPropertyL(id, *isPresentKey);
            if (isPresentPropEntry || ETrue) {
                // Get IsActive
                Usif::CIntPropertyEntry* isActivePropEntry
                                 =  (Usif::CIntPropertyEntry*) m_SCRClient.GetComponentPropertyL(id, *isActiveKey);
                if (isActivePropEntry)
                    isWidgetRunning  = (TBool) isActivePropEntry->IntValue();
            }
            break;
        }
    }  
    m_SCRClient.Close();
    
    CleanupStack::PopAndDestroy(procIDKey);
    CleanupStack::PopAndDestroy(isPresentKey);
    CleanupStack::PopAndDestroy(isActiveKey);
    CleanupStack::PopAndDestroy(appIDKey);
    CleanupStack::PopAndDestroy(globalId);
    return isWidgetRunning;
#else
    return EFalse;
#endif
}

