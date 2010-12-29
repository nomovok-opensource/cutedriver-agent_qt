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

//xml strings
const char* const VERSION = "version";
const char* const DATE_TIME = "dateTime";
const char* const NAME = "name";
const char* const TYPE = "type";
const char* const ENV = "env";
const char* const DATA_TYPE = "dataType";
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
TasAttribute::TasAttribute()
{    
}

/*!
    Destructor
*/
TasAttribute::~TasAttribute()
{    
    values.clear();            
}

/*!
    
    Set the name for the attribute. Can be for example "text" for a label object.
    Names are used to get the values for a certain object.

*/
void TasAttribute::setName(const QString& name)
{
    this->name = name; 
}

/*!
  Set the type attribute for the attribute object.
*/
void TasAttribute::setType(const QString& type)
{
    this->type = type; 
}

/*!
  Set the data type attribute for the attribute object.
*/
void TasAttribute::setDataType(const QString& type)
{
    this->dataType = type; 
}



/*!
    
    Add the attribute value. One name can be mapped to multiple values. 

*/
void TasAttribute::addValue(const QString& value)
{
    values << value;
}
void TasAttribute::addValue(const QPoint& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y();
    values << attr;
}
void TasAttribute::addValue(const QPointF& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y();
    values << attr;
}
void TasAttribute::addValue(const QSize& value)
{
    QString attr;
    QTextStream(&attr) << value.width() << "," << value.height();
    values << attr;
}
void TasAttribute::addValue(const QSizeF& value)
{
    QString attr;
    QTextStream(&attr) << value.width() << "," << value.height();
    values << attr;
}
void TasAttribute::addValue(const QRect& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y() << "," << value.width() << "," << value.height();
    values << attr;
}
void TasAttribute::addValue(const QRectF& value)
{
    QString attr;
    QTextStream(&attr) << value.x() << "," << value.y() << "," << value.width() << "," << value.height();
    values << attr;
}

/*!

    Seralize TasAttribute to xml format. Created the element using the given dom document.
    Returns the created element as is and not appended to any element inside the doc.

*/
void TasAttribute::serializeIntoString(TasXmlWriter& xmlWriter ,SerializeFilter& /*filter*/)
{  
    QMap<QString, QString> attributes;
    attributes[NAME] = name;
    if(!type.isEmpty()){
        attributes[TYPE] = type;
    }
    if(!dataType.isEmpty()){
        attributes[DATA_TYPE] = dataType;
    }
    xmlWriter.openElement(ATTRIBUTE_NAME, attributes);

    if (values.size() > 0){
        for (int i = 0; i < values.size(); ++i) {
            //            xmlWriter.openElement(VALUE_NAME);
            xmlWriter.addTextContent(values.at(i));
            //            xmlWriter.closeElement(VALUE_NAME);
        }       
    }
    xmlWriter.closeElement(ATTRIBUTE_NAME);
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
TasObject::TasObject()
{
    setEnv("qt");
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
    this->id = itemId;
}

/*!
    
    Set the type for the object. 

*/
void TasObject::setType(const QString& type)
{
    this->type = type;
}

QString TasObject::getType()
{
    return type;
}

void TasObject::setEnv(const QString& env)
{
    this->env = env;
}

/*!
    
    Set the name for the attribute. Names are very usefull since they can be used
    to identify objects. For instance a text edit field that takes the name 
    field from a form type ui should be named as "name". This makes testing
    easier since the control can now be identified as a name edit field. 

*/
void TasObject::setName(const QString& name)
{
    this->name = name;
}


/*!

    Add new attribute to the object.

*/
TasAttribute& TasObject::addAttribute()
{
    TasAttribute* attribute = new TasAttribute(); 
    attributes.append(attribute);
    return *attribute;
}

/*!

    Add new attribute to the object with the given name and value.

*/
TasAttribute& TasObject::addAttribute(const QString& name, const QString& value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value);    
    return attribute;
}

/*!
    Add new attribute to the object with the given name and value.
*/
TasAttribute& TasObject::addAttribute(const QString& name, int value)
{
    return addAttribute(name, QString::number(value));
}

/*!
    Add new attribute to the object with the given name and value.
*/
TasAttribute& TasObject::addAttribute(const QString& name, qreal value)
{
    return addAttribute(name, QString::number(value));
}

/*!
  Add a QSize as a new attribute. Format will be "width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QSize& value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value);    
    return attribute;
}

/*!
  Add a QSizeF as a new attribute. Format will be "width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QSizeF& value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value);    
    return attribute;
}

/*!
  Add a QRect as a new attribute. Format will be "x,y,width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QRect& value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value);    
    return attribute;
}


/*!
  Add a QRectF as a new attribute. Format will be "x,y,width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QRectF& value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value);    
    return attribute;
}

/*!
  Add a QRectF as a new attribute. Format will be "x,y,width,height"
 */
TasAttribute& TasObject::addAttribute(const QString& name, const QPoint& value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value);    
    return attribute;
}

/*!
  Add a QRectF as a new attribute. Format will be "x,y,width,height"
*/
TasAttribute& TasObject::addAttribute(const QString& name, const QPointF& value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value);    
    return attribute;
}



/*!

    Add new attribute to the object with the given name and value.

*/
TasAttribute& TasObject::addBooleanAttribute(const QString& name, bool value)
{
    TasAttribute& attribute = addAttribute();
    attribute.setName(name);
    attribute.addValue(value ? "true" : "false");    
    return attribute;
}


/*!

    Add new object under this object

*/
TasObject& TasObject::addObject()
{
    TasObject* object = new TasObject(); 
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
    this->parentId = parentId;
}

/*!

    Serialize TasObject to xml format. Created the element using the given dom document.
    Returns the created element as is and not appended to any element inside the doc.

*/
void TasObject::serializeIntoString(TasXmlWriter& xmlWriter ,SerializeFilter& filter)
{ 
    QMap<QString, QString> attrs;
    attrs[ID] = id; 
    attrs[NAME] = name;
    attrs[TYPE] = type;     
    if(!env.isEmpty()){
        attrs[ENV] = env;   
    }  
    if(!parentId.isEmpty()){
        attrs[PARENT] = parentId;     
    }
    xmlWriter.openElement(OBJECT_NAME, attrs);
    if (attributes.size() > 0){
        //        xmlWriter.openElement(ATTRIBUTES_NAME);
        for (int i = 0; i < attributes.size(); ++i) {
            TasAttribute* attr = attributes.at(i);
            if( filter.serializeAttribute(*attr)){
                attr->serializeIntoString(xmlWriter, filter);
            }
        }
        //        xmlWriter.closeElement(ATTRIBUTES_NAME);
    }
    if (objects.size() > 0){
        //xmlWriter.openElement(OBJECTS_NAME);
        for (int i = 0; i < objects.size(); ++i) {
            TasObject* object = objects.at(i);
            if(filter.serializeObject(*object)){                
                object->serializeIntoString(xmlWriter, filter);
            }
        }
        //xmlWriter.closeElement(OBJECTS_NAME);
    }
    xmlWriter.closeElement(OBJECT_NAME);
}


/*!

    \class TasObjectContainer
    \brief TasObjectContainer represents an entity where the object belong to
    
  
    The container can represent an application for instance that contains 
    the objects e.g Avkon/Qt.
     
*/

TasObjectContainer::TasObjectContainer()
{    
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
    this->id = QString::number(id);
}

void TasObjectContainer::setId(QString id)
{
    this->id = id;
}


/*!
    
    Set the name for the container. Can be for example "text" for a label object.
    Names are used to get the values for a certain object.

*/
void TasObjectContainer::setName(const QString& name)
{
    this->name = name;
}


/*!
    
    Set the type for the container 

*/
void TasObjectContainer::setType(const QString& type)
{
    this->type = type;
}

/*!

    Add a new object to the container.

*/
TasObject& TasObjectContainer::addNewObject()
{    
    TasObject* object = new TasObject(); 
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

    Seralize TasObjectContainer to xml format. Created the element using the given dom document.
    Returns the created element as is and not appended to any element inside the doc.

*/
void TasObjectContainer::serializeIntoString(TasXmlWriter& xmlWriter, SerializeFilter& filter, bool elementsOnly)
{
    if(!elementsOnly){
        QMap<QString, QString> attributes;
        attributes[ID] = id; 
        attributes[NAME] = name; 
        attributes[TYPE] = type;     
        xmlWriter.openElement(CONTAINER_NAME, attributes);
    }
    for (int i = 0; i < objects.size(); ++i) {
        TasObject* object = objects.at(i);
        if(filter.serializeObject(*object)){
            object->serializeIntoString(xmlWriter, filter);
        }
    }
    if(!elementsOnly){
        xmlWriter.closeElement(CONTAINER_NAME);
    }
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
{}

/*!
    Destructor
 */
TasDataModel::~TasDataModel()
{    
    clearModel();
}


/*!
  Clears the model.
*/
void TasDataModel::clearModel()
{
    qDeleteAll(containers);
    containers.clear();        
}

/*!

    Adds a new TasObjectContainer to the model. The model 
    can contain more that one container. Use the container
    to add objects to the model. 

*/
TasObjectContainer& TasDataModel::addNewObjectContainer()
{
    TasObjectContainer* container = new TasObjectContainer();
    containers.append(container);
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
    for (int i = 0; i < containers.size(); ++i) {
        TasObjectContainer* container = containers.at(i);
        if(container->id == id){
            match = container;
            break;
        }
    }
    return match;
}

/*!

    Serialize the model into xml format specified by testabililty.
    Filter ownership is assumed and it will be removed once
    the serializing has been completed.
*/
void TasDataModel::serializeModel(QByteArray& xmlData, SerializeFilter* filter, bool containers)
{
    if(!filter){
        filter = new SerializeFilter();
    }
    QTextStream stream(&xmlData, QIODevice::WriteOnly);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    TasXmlWriter tasXmlWriter(stream);
    if(containers){
        serializeObjects(tasXmlWriter, *filter, true);
    }
    else{
        serializeIntoString(tasXmlWriter, *filter);
    }
    delete filter;
}

void TasDataModel::serializeIntoString(TasXmlWriter& xmlWriter, SerializeFilter& filter)
{
    QMap<QString, QString> attributes;
    attributes[VERSION] = TAS_VERSION; 
    xmlWriter.openElement(ROOT_NAME, attributes);
    serializeObjects(xmlWriter, filter);
    xmlWriter.closeElement(ROOT_NAME);
}


void TasDataModel::serializeObjects(TasXmlWriter& xmlWriter, SerializeFilter& filter, bool elementsOnly)
{
    for (int i = 0; i < containers.size(); ++i) {
        TasObjectContainer* container = containers.at(i);
        if ( filter.serializeContainer(*container)){
            container->serializeIntoString(xmlWriter, filter, elementsOnly);
        }
    }       
}

/*!

    \class SerializeFilter
    \brief Allows to filter some elements from the xml tree

    The SerializeFilter makes it possible to set filters to avoid
    the models into producing too much information and therefore 
    slowing thing down. The metaproperty system will at time 
    produce a lot of information that may not always be what is 
    needed. 

*/

/*!
  
    Constructor 
  
 */
SerializeFilter::SerializeFilter()
{
    allowDuplicates = true; //default
}

SerializeFilter::~SerializeFilter()
{
    serializedObjects.clear();
}

/*!
   
    Set the filter to block or allow duplicates.

*/
void SerializeFilter::serializeDuplicates(bool allow)
{
    allowDuplicates = allow;
}

/*!
  
  Checks the attribute and returns false if the attribute should not 
  be added to the model.
    
 */
bool SerializeFilter::serializeAttribute(TasAttribute& /*attribute*/)
{
    return true;
}

/*!
  
  Checks the container and returns false if the container should not 
  be added to the model.
    
 */
bool SerializeFilter::serializeContainer(TasObjectContainer& /*container*/)
{
    return true;
}

/*!
  
  Checks the object and returns false if the object should not 
  be added to the model.
  Default implementation check duplicates.
    
 */
bool SerializeFilter::serializeObject(TasObject& object)
{        
    bool serialize = true;
    if(!serializedObjects.contains(object.id)){       
        serializedObjects << object.id;        
    }
    else if(!allowDuplicates){
        serialize = false;        
    }    
    return serialize;
}




