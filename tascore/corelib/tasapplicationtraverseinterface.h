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
 


#ifndef TASAPPLICATIONTRAVERSEINTERFACE_H
#define TASAPPLICATIONTRAVERSEINTERFACE_H

#include <QByteArray>
#include <QString>

/*!
    \class TasApplicationTraverseInterface
    \brief TasApplicationTraverseInterface provides a method to traverse os specific application details.
    
	TasApplicationTraverseInterface implementations will be loaded by the qttasserver when ui state is requested.
	Implementing classes can add platform specific details to the ui state. The details provided will be added to
	the normal qt ui state.
	
*/
class TasApplicationTraverseInterface
{
 public:
     virtual ~TasApplicationTraverseInterface() {}
   
	 virtual QByteArray traverseApplication(const QString& pid, const QString& name, const QString& applicationUid) = 0;
};

Q_DECLARE_INTERFACE(TasApplicationTraverseInterface,
					"com.nokia.testability.TasApplicationTraverseInterface/1.0")

#endif 
 
