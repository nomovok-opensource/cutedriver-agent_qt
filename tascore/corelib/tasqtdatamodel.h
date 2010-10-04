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
 


#ifndef TASTDATAMODEL_H
#define TASTDATAMODEL_H

#include <QList>
#include <QSize>
#include <QSizeF>
#include <QRect>
#include <QRectF>
#include <QStringList>
#include <QTextStream>

#include "tasconstants.h"

class TasAttribute;
class TasObject;
class TasObjectContainer;
class TasDataModel;
class TasXmlWriter;

class TAS_EXPORT SerializeFilter
{
public:
    SerializeFilter(); 
    virtual ~SerializeFilter();
    
    virtual bool serializeObject(TasObject& object);
    virtual bool serializeAttribute(TasAttribute& attribute);
    virtual bool serializeContainer(TasObjectContainer& container);
    virtual void serializeDuplicates(bool allow);
    
private:
    QStringList serializedObjects;

    bool allowDuplicates;
};

class TAS_EXPORT TasAttribute 
{
private:
    TasAttribute();
    
public:    
    ~TasAttribute();

    void setName(const QString& name);    
	void setType(const QString& type);    
	void setDataType(const QString& type);    
    void addValue(const QString& value);    
    void addValue(const QSize& value);    
    void addValue(const QSizeF& value);    
    void addValue(const QPoint& value);    
    void addValue(const QPointF& value);    
    void addValue(const QRect& value);    
    void addValue(const QRectF& value);    
    
private:
    void serializeIntoString(TasXmlWriter& xmlWriter, SerializeFilter& filter);

private:    
    QString name;    
    QString type;    
    QString dataType;    
    QStringList values;
    
private:
    friend class SerializeFilter;
    friend class TasObject;
    friend class TasObjectContainer;
};

class TAS_EXPORT TasObject 
{
private:
    TasObject();
    
public:    
    ~TasObject();

    TasAttribute& addAttribute();
    TasAttribute& addAttribute(const QString& name, const QString& value);
    TasAttribute& addAttribute(const QString& name, int value);    
    TasAttribute& addAttribute(const QString& name, qreal value);    
    TasAttribute& addAttribute(const QString& name, const QSize& value);    
    TasAttribute& addAttribute(const QString& name, const QSizeF& value);    
    TasAttribute& addAttribute(const QString& name, const QRect& value);    
    TasAttribute& addAttribute(const QString& name, const QRectF& value);    
    TasAttribute& addAttribute(const QString& name, const QPoint& value);    
    TasAttribute& addAttribute(const QString& name, const QPointF& value);    
    TasAttribute& addBooleanAttribute(const QString& name, bool value);
    TasObject& addObject();
    TasObject& addNewObject(QString id, const QString& name, const QString& type);
    void setId(const QString& id);
    void setName(const QString& name);
    void setType(const QString& type);
	QString getType();
    void setParentId(const QString& id );
        
private:
    void serializeIntoString(TasXmlWriter& xmlWriter, SerializeFilter& filter);    

private:
    QList<TasAttribute*> attributes;
    QList<TasObject*> objects;
    QString id;
    QString parentId;
    QString type;    
    QString name;
    
    friend class TasObjectContainer; 
    friend class SerializeFilter;
};

class TAS_EXPORT TasObjectContainer 
{
private:
    TasObjectContainer();
    
public:
    ~TasObjectContainer();

    void setId(int id);
    void setName(const QString& name);
    void setType(const QString& type);
    TasObject& addNewObject();
    TasObject& addNewObject(const QString& id, const QString& name, const QString& type);

private:
    void serializeIntoString(TasXmlWriter& xmlWriter, SerializeFilter& filter);    

private:
    QString id;
    QString name;
    QString type;    
    QList<TasObject*> objects;
    
    friend class TasDataModel;    
    friend class SerializeFilter;
};

class TAS_EXPORT TasDataModel  
{    
public:
    TasDataModel();
    ~TasDataModel();         
   
public:    
    TasObjectContainer& addNewObjectContainer();
    TasObjectContainer& addNewObjectContainer(const QString& name, const QString& type);
    TasObjectContainer& addNewObjectContainer(int id, const QString& name, const QString& type);	

    QString serializeModel(SerializeFilter* filter = 0);    
    void serializeModel(QByteArray& xmlData, SerializeFilter* filter = 0);    
	void clearModel();

	TasObjectContainer* findObjectContainer(const QString& id);

private:
    void serializeIntoString(TasXmlWriter& xmlWriter, SerializeFilter& filter);

private:
    QList<TasObjectContainer*> containers;
    friend class SerializeFilter;
        
};


#endif
