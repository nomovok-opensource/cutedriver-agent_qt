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
#include <QtPlugin>
#include <QHashIterator>
#include <QFile>

#ifdef Q_OS_SYMBIAN
#include <e32base.h>
#include <e32hashtab.h>
#include <TasFixturePluginInterface.h>
#include <xqconversions.h>
// FUNCTION PROTOTYPES
TUint32 RBufHashFunction( const RBuf8& aObject );
TBool RBufIdentityFunction( const RBuf8& aObject1, const RBuf8& aObject2 );

#endif

#include "sdfixture.h"



Q_EXPORT_PLUGIN2(sdfixture, SdFixture)

/*!
  \class SdFixture
  \brief Collects frames per second data from widgets.
        
  Filters paint events to the given target and calculates fps.
  For graphicsitems the view's viewport is used.
*/

/*!
  Constructor
*/
SdFixture::SdFixture(QObject* parent)
    :QObject(parent)
{
    debug("SdFixture::created");
}

/*!
  Destructor
*/
SdFixture::~SdFixture()
{
}

/*!
  Implementation for traverse so always true.
*/
bool SdFixture::execute(void * objectInstance, QString actionName, QHash<QString, QString> parameters, QString & stdOut)
{
    debug("SdFixture::execute");
    bool returnValue = true;
#ifdef Q_OS_SYMBIAN
    const TUid fixtureUid = TUid::Uid(0x20026F7E);    
    THashFunction32<RBuf8> hashFunc( RBufHashFunction );
    TIdentityRelation<RBuf8> idFunc( RBufIdentityFunction );
    RHashMap<RBuf8, RBuf8> paramPairs( hashFunc, idFunc );
    CleanupClosePushL( paramPairs );
    debug("SdFixture::execute read params");
    QHashIterator<QString, QString> i(parameters);
    while (i.hasNext()) {
        i.next();
        if(!i.key().isEmpty() && !i.value().isEmpty() && i.key() != OBJECT_TYPE){
            debug("parametrit: " + i.key()+";"+ i.value());
            debug("SdFixture::execute read key");
            RBuf8 keyBuf;
            RBuf8 valueBuf;
            TPtrC16 keyStr(reinterpret_cast<const TUint16*>(i.key().utf16()));
            TPtrC16 valueStr(reinterpret_cast<const TUint16*>(i.value().utf16()));
            keyBuf.Create(keyStr.Length());
            valueBuf.Create(valueStr.Length());
            keyBuf.Copy(keyStr);
            valueBuf.Copy(valueStr);
            debug("SdFixture::execute insert to hash");    
            paramPairs.InsertL( keyBuf, valueBuf );
            debug("SdFixture::execute pop hbufs");    
        }
    }
    debug("SdFixture::execute make fixture");
    CTasFixturePluginInterface* fixture = CTasFixturePluginInterface::NewL( fixtureUid );
    CleanupStack::PushL( fixture );
    debug("SdFixture::execute conver actionname");
    TPtrC16 actionStr(reinterpret_cast<const TUint16*>(actionName.utf16()));
    RBuf8 actionBuf;
    actionBuf.Create(actionStr.Length());
    CleanupClosePushL( actionBuf );
    actionBuf.Copy(actionStr);
    debug("SdFixture::execute execute fixture");
    RBuf8 response;
    CleanupClosePushL( response );
    if(fixture->Execute( NULL, actionBuf, paramPairs, response ) != KErrNone){
        returnValue = false;
    }
    debug("SdFixture::execute convert response");
    stdOut  = XQConversions::s60Desc8ToQString(response);
    CleanupStack::PopAndDestroy( 4 );//response, fixture, paramPairs
#endif    
    debug("SdFixture::execute done");
    return returnValue;
}

void SdFixture::debug(QString line)
{
    QFile out("e:\\logs\\testability\\fixture.log");
    out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    out.write(line.toAscii());
    out.write("\n");
    out.flush();
    out.close();
}

#ifdef Q_OS_SYMBIAN
TUint32 RBufHashFunction( const RBuf8& aObject )
    {
    return DefaultHash::Integer( aObject.Length() );
    }
    
TBool RBufIdentityFunction( const RBuf8& aObject1, const RBuf8& aObject2 )
    {
    return aObject1 == aObject2;
    }
#endif
