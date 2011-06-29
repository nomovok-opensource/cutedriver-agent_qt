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
#include "tasqtcommandmodel.h"


class TasAttribute;
class TasObject;
class TasObjectContainer;
class TasDataModel;
class TasXmlWriter;


/*!
  This class is depracated and is no longer used anywhere.
  Kept for compatibility issues.
*/
class TAS_EXPORT SerializeFilter
{
public:
    virtual ~SerializeFilter(){}
    virtual bool serializeObject(TasObject& object){return true;}
    virtual bool serializeAttribute(TasAttribute& attribute){return true;}
    virtual bool serializeContainer(TasObjectContainer& container){return true;}
	virtual void serializeDuplicates(bool allow){}
};


class TAS_EXPORT TasAttribute : public TasDomObject
{
private:
    TasAttribute(QDomElement element);
    TasAttribute(QDomElement element, const QString& name);
    
public:    
    ~TasAttribute();

    void setName(const QString& name);    
    void setType(const QString& type);    
    void setDataType(const QString& type);    
    void addValue(const QString& value);
    void addValuePlainString(const QString& value); // doesn't call TasCoreUtils::encodeString
    void addValue(const QSize& value);    
    void addValue(const QSizeF& value);    
    void addValue(const QPoint& value);    
    void addValue(const QPointF& value);    
    void addValue(const QRect& value);    
    void addValue(const QRectF& value);    
    
private:    
    QString name;    
    QString type;    
    QString dataType;    
    QStringList values;
    
private:
    friend class TasObject;
    friend class TasObjectContainer;
};

class TAS_EXPORT TasObject : public TasDomObject
{
private:
    TasObject(QDomElement element);
    
public:    
    ~TasObject();

    TasAttribute& addAttribute();
    TasAttribute& addAttribute(const QString& name);
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
	void setEnv(const QString& env);
	QString getType();
    void setParentId(const QString& id );
     
private:
    QList<TasAttribute*> attributes;
    QList<TasObject*> objects;
    friend class TasObjectContainer; 
};

class TAS_EXPORT TasObjectContainer : public TasDomObject
{
private:
    TasObjectContainer(QDomElement element);
    
public:
    ~TasObjectContainer();

    void setId(int id);
    void setId(QString id);
    void setName(const QString& name);
    void setType(const QString& type);
    TasObject& addNewObject();
    TasObject& addNewObject(const QString& id, const QString& name, const QString& type);

private:
    QList<TasObject*> objects;   
    friend class TasDataModel;    
};

class TAS_EXPORT TasDataModel : public TasDomObject
{    
public:
    TasDataModel();
    ~TasDataModel();         
   
public:    
    TasObjectContainer& addNewObjectContainer();
    TasObjectContainer& addNewObjectContainer(const QString& name, const QString& type);
    TasObjectContainer& addNewObjectContainer(int id, const QString& name, const QString& type);	
	
    void serializeModel(QByteArray& xmlData, SerializeFilter* filter, bool containers=false);    
    void serializeModel(QByteArray& xmlData, bool containers=false);    
	void clearModel();

	TasObjectContainer* findObjectContainer(const QString& id);

private:
	QDomDocument mDocument;
    QList<TasObjectContainer*> mContainers;
       
};


#endif
