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

#include "taspointercache.h"

TasPointerCache *TasPointerCache::mInstance = 0;

TasPointerCache::TasPointerCache()
{}

TasPointerCache* TasPointerCache::instance()
{
    if(mInstance == 0){
        mInstance = new TasPointerCache();
    }
    return mInstance;
}

QObject* TasPointerCache::getObject(const QString& pointerId)
{    
    QPointer<QObject> pointer = mPointers.value(pointerId, 0);    
    if(pointer.isNull()){
        mPointers.remove(pointerId);        
        return 0;
    }
    return pointer.data();
}

void TasPointerCache::storePointer(const QString& pointerId, QObject* pointer)
{
    mPointers.insert(pointerId, QPointer<QObject>(pointer));
}

void TasPointerCache::removeNulls()
{
    QMutableHashIterator<QString, QPointer<QObject> > i(mPointers);
    while (i.hasNext()) {
        i.next();
        QPointer<QObject> pointer = i.value();
        if(pointer.isNull()){
            i.remove();
        }
    }
}
