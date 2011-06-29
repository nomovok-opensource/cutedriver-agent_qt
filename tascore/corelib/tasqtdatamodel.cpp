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

#include <QDebug>
#include <QDateTime>
#include <QMutableListIterator>
#include <QTextCodec>

#include "tasxmlwriter.h"
#include "version.h"
#include "tasqtdatamodel.h"
#include "taslogger.h"
#include <tascoreutils.h>

//xml strings
const char* const VERSION = "version";
//const char* const DATE_TIME = "dateTime";
const char* const NAME = "name";
const char* const TYPE = "type";
const char* const ENV = "env";
const char* const DATA_TYPE = "type";
const char* const ACCESS = "access";
const char* const ID = "id";
const char* const ROOT_NAME = "tasMessage";
const char* const CONTAINER_NAME = "tasInfo";
const char* const OBJECT_NAME = "obj";
const char* const ATTRIBUTE_NAME = "attr";
const char* const PARENT = "parent";


/*!
    \class TasAttribute
    \brief TasAttribute represents an attribute that an object can have.     
    

    Represents an attribute in tas data 
    model. Attributes consist of name and value pairs.
  
    Attribute in the tas data model represents an attribute of an object.
    Attributes can be simple name value pairs.
       
    
 */

/*!
 
    Constructor for TasAttribute. Attributes must be created through the 
    TasObject instance that they belong to.
    
*/
TasAttribute::TasAttribute(QDomElement element)
{    
    mElement = element;
}

TasAttribute::TasAttribute(QDomElement element, const QString& name)
{    
    mElement = element;
    addDomAttribute(NAME, name);
}

/*!
    Destructor
*/
TasAttribute::~TasAttribute()
{    
}

/*!
    
    Set the name for the attribute. Can be for example "text" for a label object.
    Names are used to get the values for a certain object.

*/
void TasAttribute::setName(const QString& name)
{
    addDomAttribute(NAME, name);
}

/*!
  Set the type attribute for the attribute object.
*/
void TasAttribute::setType(const QString& type)
{
    addDomAttribute(ACCESS, type);
}

/*!
  Set the data type attribute for the attribute object.
*/
void TasAttribute::setDataType(const QString& type)
{
    addDomAttribute(DATA_TYPE, type);
}


/*!
    
    Add the attribute value. One name can be mapped to multiple values. 

*/
void TasAttribute::addValue(const QString& value) //, bool encodeString)
{
    setText(value);
}

void TasAttribute::addValuePlainString(const QString& value)
{
    setText(value);
}

void TasAttribute::addValue(const QPoint& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y();
    setText(attr);
}
void TasAttribute::addValue(const QPointF& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y();
    setText(attr);
}
void TasAttribute::addValue(const QSize& value)
{
    QString attr;
    QTextStream(&attr) << value.width() << "," << value.height();
    setText(attr);
}
void TasAttribute::addValue(const QSizeF& value)
{
    QString attr;
    QTextStream(&attr) << value.width() << "," << value.height();
    setText(attr);
}
void TasAttribute::addValue(const QRect& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y() << "," << value.width() << "," << value.height();
    setText(attr);
}
void TasAttribute::addValue(const QRectF& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y() << "," << value.width() << "," << value.height();
    setText(attr);
}

/*!
    \class TasObject
    \brief TasObject represents an object in the tas data model     
    

    Represents an object in tas data model. 
    The object does not have to contain all elements.        
    
 */

/*!
 
    Constructor for TasObject. Objects must be created through the 
    TasContainer instance that they belong to.
    
*/
TasObject::TasObject(QDomElement element)
{
    mElement = element;
    addDomAttribute(ENV, "qt");
}

/*!
    Destructor
*/
TasObject::~TasObject()
{
    qDeleteAll(attributes);
    attributes.clear();
    qDeleteAll(objects);    
    objects.clear(); 
}


/*!

    Set an id to the object. Ids should be as uniques as possible.
    Ids are used to separate objects. 

*/
void TasObject::setId(const QString& itemId)
{
    addDomAttribute(ID, itemId);
}

/*!
    
    Set the type for the object. 

*/
void TasObject::setType(const QString& type)
{
    addDomAttribute(TYPE, type);
}

QString TasObject::getType()
{
    return parameter(TYPE);
}

void TasObject::setEnv(const QString& env)
{
    addDomAttribute(ENV, env);
}

/*!
    
    Set the name for the attribute. Names are very usefull since they can be used
    to identify objects. For instance a text edit field that takes the name 
    field from a form type ui should be named as "name". This makes testing
    easier since the control can now be identified as a name edit field. 

*/
void TasObject::setName(const QString& name)
{
    addDomAttribute(NAME, name);
}


/*!

    Add new attribute to the object.

*/
TasAttribute& TasObject::addAttribute()
{
    QDomElement element = addChild(ATTRIBUTE_NAME);
    TasAttribute* attribute = new TasAttribute(element); 
    attributes.append(attribute);
    return *attribute;
}

/*!

    Add new attribute to the object with name.

*/
TasAttribute& TasObject::addAttribute(const QString& name)
{
    QDomElement element = addChild(ATTRIBUTE_NAME);
    TasAttribute* attribute = new TasAttribute(element, name); 
    attributes.append(attribute);
    return *attribute;
}

/*!

    Add new attribute to the object with the given name and value.

*/
TasAttribute& TasObject::addAttribute(const QString& name, const QString& value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value);
    return attribute;

}

/*!
    Add new attribute to the object with the given name and value.
*/
TasAttribute& TasObject::addAttribute(const QString& name, int value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(QString::number(value));
    return attribute;
}

/*!
    Add new attribute to the object with the given name and value.
*/
TasAttribute& TasObject::addAttribute(const QString& name, qreal value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValuePlainString(QString::number(value));
    return attribute;

}

/*!
  Add a QSize as a new attribute. Format will be "width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QSize& value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value);
    return attribute;
}

/*!
  Add a QSizeF as a new attribute. Format will be "width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QSizeF& value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value);
    return attribute;
}

/*!
  Add a QRect as a new attribute. Format will be "x,y,width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QRect& value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value);
    return attribute;
}


/*!
  Add a QRectF as a new attribute. Format will be "x,y,width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QRectF& value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value);
    return attribute;
}

/*!
  Add a QRectF as a new attribute. Format will be "x,y,width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QPoint& value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value);
    return attribute;
}

/*!
  Add a QRectF as a new attribute. Format will be "x,y,width,height"
*/
TasAttribute& TasObject::addAttribute(const QString& name, const QPointF& value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value);
    return attribute;
}

/*!

    Add new attribute to the object with the given name and value.

*/
TasAttribute& TasObject::addBooleanAttribute(const QString& name, bool value)
{
    TasAttribute& attribute = addAttribute(name);
    attribute.addValue(value ? "true" : "false");
    return attribute;
}

/*!

    Add new object under this object

*/
TasObject& TasObject::addObject()
{
    QDomElement element = addChild(OBJECT_NAME);
    TasObject* object = new TasObject(element); 
    objects.append(object);
    return *object;
}

/*!

    Add new object under this object with the given object values.

*/
TasObject& TasObject::addNewObject(QString id, const QString& name, const QString& type)
{
    TasObject& object = addObject();
    object.setId(id);
    object.setName(name);
    object.setType(type);
    return object;
}

void TasObject::setParentId(const QString& parentId )
{
    addAttribute(PARENT, parentId);
}

/*!

    \class TasObjectContainer
    \brief TasObjectContainer represents an entity where the object belong to
    
  
    The container can represent an application for instance that contains 
    the objects e.g Avkon/Qt.
     
*/

TasObjectContainer::TasObjectContainer(QDomElement element)
{    
    mElement = element;
}

TasObjectContainer::~TasObjectContainer()
{
    qDeleteAll(objects);
    objects.clear();        
}


/*!

    Set an id to the container. Ids should be as uniques as possible.
    Ids are used to separate objects and attributes. 

*/
void TasObjectContainer::setId(int id)
{
    addDomAttribute(ID, QString::number(id));
}

void TasObjectContainer::setId(QString id)
{
    addDomAttribute(ID, id);
}

/*!
    
    Set the name for the container. Can be for example "text" for a label object.
    Names are used to get the values for a certain object.

*/
void TasObjectContainer::setName(const QString& name)
{
    addDomAttribute(NAME, name);
}


/*!
    
    Set the type for the container 

*/
void TasObjectContainer::setType(const QString& type)
{
    addDomAttribute(TYPE, TYPE);
}

/*!

    Add a new object to the container.

*/
TasObject& TasObjectContainer::addNewObject()
{
    QDomElement element = addChild(OBJECT_NAME);
    TasObject* object = new TasObject(element);     
    objects.append(object);
    return *object;
}

/*!

    Add a new object to the container with the given values.

*/
TasObject& TasObjectContainer::addNewObject(const QString& id, const QString& name, const QString& type)
{
    TasObject& object = addNewObject();
    object.setId(id);
    object.setName(name);
    object.setType(type);
    return object;
}

/*!

    \class TasDataModel
    \brief TasDataModel Root class of the data model

 
    Interface to the TasDataModel. The tas data model contains the ui data collected 
    from qt. 

*/

/*!
  
    Constructor for the TasDataModel. The model is created completely empty. 
  
 */
TasDataModel::TasDataModel()
    :mDocument(ROOT_NAME)
{
    QDomImplementation::setInvalidDataPolicy(QDomImplementation::DropInvalidChars);
    mElement = mDocument.createElement(ROOT_NAME);
    mDocument.appendChild(mElement);
    addDomAttribute(VERSION, TAS_VERSION);
}

/*!
    Destructor
 */
TasDataModel::~TasDataModel()
{    
    qDeleteAll(mContainers);
}


/*!
  Clears the model.
*/
void TasDataModel::clearModel()
{
    qDeleteAll(mContainers);
    mContainers.clear();        
    mDocument.removeChild(mElement);
    mElement.clear();
    mElement = mDocument.createElement(ROOT_NAME);
    mDocument.appendChild(mElement);
}

/*!

    Adds a new TasObjectContainer to the model. The model 
    can contain more that one container. Use the container
    to add objects to the model. 

*/
TasObjectContainer& TasDataModel::addNewObjectContainer()
{
    QDomElement element = addChild(CONTAINER_NAME);
    TasObjectContainer* container = new TasObjectContainer(element);
    mContainers.append(container);
    return *container;
}

/*!

    Adds a new container to the model with the given values.
    
*/
TasObjectContainer& TasDataModel::addNewObjectContainer(const QString& name, const QString& type)
{
    TasObjectContainer& container = addNewObjectContainer();
    container.setName(name);
    container.setType(type);    
    return container;
}


/*!

    Adds a new container to the model with the given values.
    
*/
TasObjectContainer& TasDataModel::addNewObjectContainer(int id, const QString& name, const QString& type)
{
    TasObjectContainer& container = addNewObjectContainer();
    container.setId(id);
    container.setName(name);
    container.setType(type);    
    return container;
}

/*!
  Look for a container with the given id. The first occurence of a container with
  the given id is placed in the container. Returns false if no matches found.
 */
TasObjectContainer* TasDataModel::findObjectContainer(const QString& id)
{
    TasObjectContainer* match = 0;
    for (int i = 0; i < mContainers.size(); ++i) {
        TasObjectContainer* container = mContainers.at(i);
        if(container->parameter(ID) == id){
            match = container;
            break;
        }
    }
    return match;
}

/*!
  DEPRACATED, use TasDataModel::serializeModel(QByteArray& xmlData, bool containers)
*/
void TasDataModel::serializeModel(QByteArray& xmlData, SerializeFilter* /*filter*/, bool containers)
{
    TasLogger::logger()->warning("TasDataModel::serializeModel this method is depracated.");
    TasLogger::logger()->warning("Use TasDataModel::serializeModel(QByteArray& xmlData, bool mContainers)");
    serializeModel(xmlData, containers);
}

/*!
  Serializes the model or only containers (including children).
 */
void TasDataModel::serializeModel(QByteArray& xmlData, bool containers)
{
    //only containers
    if(containers){
        foreach(TasObjectContainer* container, mContainers){
            QTextStream xmlStream(&xmlData, QIODevice::Append);
            xmlStream.setCodec("UTF-8");            
            container->domElement().save(xmlStream, -1);
        }        
    }
    else{
        xmlData = mDocument.toByteArray(-1);
    }
}

