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
 


#ifndef TASTRAVERSEINTERFACE_H
#define TASTRAVERSEINTERFACE_H


#include <QObject>
#include <QGraphicsItem>

#include "tasqtdatamodel.h"
#include "tasbasetraverse.h"

class TasCommand;

/*!
    \class TasHelperInterface
    \brief TasHelperInterface Interface definition for plugins used to traverse special components
    
    Special components that require custom traversal (e.g usage of components outside QT) can 
    build a plugin component that implements this interface to add special details to components 
    being traversed. 
*/
class TasTraverseInterface : public TasBaseTraverse
 {
 public:
     virtual ~TasTraverseInterface() {}
     
     /*!     
      Traverse the object by adding the special details to the given TasObject
      reference. All default details will be added to the TasObject data container
      by the framework.     
     */
	  virtual void traverseObject(TasObject* , QObject* , TasCommand* = 0){} 

     /*!     
      Traverse the graphicsitem by adding the special details to the given TasObject
      reference. All default details will be added to the TasObject data container
      by the framework.           
     */
	  virtual void traverseGraphicsItem(TasObject* , QGraphicsItem* , TasCommand*  = 0){}

	  /*!
		Used to identify the plugins
	   */
	  void setPluginName(const QString& pluginName){mPluginName=pluginName;}
	  QString getPluginName(){return mPluginName;}
	  
protected:
	  QString mPluginName;

};

 Q_DECLARE_INTERFACE(TasTraverseInterface,
                     "com.nokia.testability.TasTraverseInterface/1.0")

#endif 
 
