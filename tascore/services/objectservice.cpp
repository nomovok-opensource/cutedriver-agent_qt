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
#include <QListIterator>
#include <QMetaProperty>
#include <QVariant>
#include <QDateTime>

#include "objectservice.h"
#include "taslogger.h"

/*!
  \class ObjectService
  \brief ObjectService sets object properties and invoked methods.

*/

ObjectService::ObjectService()
{
}

ObjectService::~ObjectService()
{
}

bool ObjectService::executeService(TasCommandModel& model, TasResponse& response)
{
    if(model.service() == serviceName() ){
        performObjectService(model, response);
        model.forceUiUpdate(true);
        return true;
    }
    else{
        return false;
    }
}

void ObjectService::performObjectService(TasCommandModel& model, TasResponse& response)
{
    TasLogger::logger()->debug(QString("TasCommander::performObjectService entry"));
    QListIterator<TasTarget*> i(model.targetList());
    QString errorString;
    while (i.hasNext()){
        TasTarget* commandTarget = i.next();
        QString targetId = commandTarget->id();
        QString targetType = commandTarget->type();
        QObject* target = 0;

        if(targetType == TYPE_GRAPHICS_VIEW){
            QGraphicsItem* item = findGraphicsItem(targetId);
            target = TestabilityUtils::castToGraphicsWidget(item);
        }
        else if(targetType == TYPE_STANDARD_VIEW){
            target = findWidget(targetId);
        }
        else if(targetType == TYPE_APPLICATION_VIEW ){
            target = qApp;
        }
        // TODO: add support
        //else if(targetType == TYPE_ACTION_VIEW = "Action";
        //else if(targetType == TYPE_LAYOUT = "Layout";
        //else if(targetType == TYPE_LAYOUT_ITEM = "LayoutItem";
        //else if(targetType == TYPE_WEB = "Web";
        //else if(targetType == TYPE_QWEB = "QWeb";
        else {
            errorString.append(targetType + " target type not supported. ");
        }

        TasLogger::logger()->debug(
                    QString("TasCommander::performObjectService %1 %2 found: %3")
                    .arg(targetType).arg(targetId).arg(target!=NULL));

        if (target) {
            QListIterator<TasCommand*> j(commandTarget->commandList());
            while (j.hasNext()){
                TasCommand* command = j.next();
                if(command->name() == "SetAttribute"){
                    doSetAttribute(command, target, errorString);
                }
                else if (command->name() == "CallMethod"){
                    doCallMethod(command, target, errorString);
                }
                else {
                    TasLogger::logger()->debug(
                                QString("TasCommander::performObjectService %1 %2: unsupported %3")
                                .arg(targetType).arg(targetId).arg(command->name()));
                }
            }
        }
    }

    if(!errorString.isEmpty()){
        response.setErrorMessage(errorString);
        TasLogger::logger()->debug(
                    QString("TasCommander::performObjectService errors: %1")
                    .arg(errorString));
    }
    else {
        TasLogger::logger()->debug(
                    QString("TasCommander::performObjectService, no errors."));
    }
}


/*!
Function for executing a slot that does not take parameters
*/
void ObjectService::doCallMethod(TasCommand* command, QObject* target, QString& errorString)
{
    Q_ASSERT(command->name() == "CallMethod");

    QString methodName = command->parameter("method_name");
    int methodId = target->metaObject()->indexOfMethod(
                QMetaObject::normalizedSignature(methodName.toAscii()).constData());
    TasLogger::logger()->debug(
                QString("TasCommander::performObjectService %1('%2')=('%3') methodId %4")
                .arg(command->name())
                .arg(methodName)
                .arg(QMetaObject::normalizedSignature(methodName.toAscii()).constData())
                .arg(methodId));
    if (methodId == -1){
        errorString.append(methodName + " method not found on object. ");
        TasLogger::logger()->debug("...method not found on object");
    }

    else{
        QMetaMethod metaMethod = target->metaObject()->method(methodId);
        TasLogger::logger()->debug("...got metaMethod");
        if(!metaMethod.invoke(target, Qt::DirectConnection)){
            errorString.append(methodName + " method invocation failed! ");
            TasLogger::logger()->debug("...invoke failed");
        }
    }
}


/*!
Function for setting attribute value - append errorString if attribute not found, not writable or it doesn't get set correctly.
*/
void ObjectService::doSetAttribute(TasCommand* command, QObject* target, QString& errorString)
{
    Q_ASSERT(command->name() == "SetAttribute");
    int propertyId = target->metaObject()->indexOfProperty(command->parameter("attribute_name").toLatin1().data());
    TasLogger::logger()->debug("ObjectService::doSetAttribute propertyid is " + QString::number(propertyId));
    TasLogger::logger()->debug("ObjectService::doSetAttribute className is " + QString(target->metaObject()->className()));

    if (propertyId == -1){
        errorString.append(command->parameter("attribute_name") +  QString(" attribute not found on object. "));
        return;
    }

    QMetaProperty attributeMetaProperty = target->metaObject()->property(propertyId);

    if (!attributeMetaProperty.isWritable()){
        errorString.append(command->parameter("attribute_name") +  QString(" attribute was not writable. "));
        return;
    }

    QString attrType = command->parameter("attribute_type");
    QVariant attrValue;
    QVariant::Type variantType = QVariant::nameToType(attrType.toAscii().data());
    if (attrType == "bool"){
        if (command->parameter("attribute_value") == "true"){
            attrValue = QVariant(true);
        }
        else{
            attrValue = QVariant(false);
        }
    }
    else if (attrType == "int"){
        attrValue = QVariant(int(command->parameter("attribute_value").toInt()));
    }
    else{
        QStringList params = command->parameter("attribute_value").split(",");
        switch(variantType)
        {
        case QVariant::Point:
            attrValue = QVariant(QPoint(params.at(0).toInt(), params.at(1).toInt()));
            break;
        case QVariant::PointF:
            attrValue = QVariant(QPointF(params.at(0).toFloat(), params.at(1).toFloat()));
            break;
        case QVariant::Size:
            attrValue = QVariant(QSize(params.at(0).toInt(), params.at(1).toInt()));
            break;
        case QVariant::SizeF:
            attrValue = QVariant(QSizeF(params.at(0).toFloat(), params.at(1).toFloat()));
            break;
        case QVariant::Rect:
            attrValue = QVariant(QRect(params.at(0).toInt(), params.at(1).toInt(), params.at(2).toInt(),params.at(3).toInt()));
            break;
        case QVariant::RectF:
            attrValue = QVariant(QRectF(params.at(0).toFloat(), params.at(1).toFloat(), params.at(2).toFloat(),params.at(3).toFloat()));
            break;
        case QVariant::DateTime:
            attrValue = QVariant(QDateTime::fromTime_t(command->parameter("attribute_value").toUInt()));
            break;
        case QVariant::Date:
            attrValue = QVariant(QDate::fromString(command->parameter("attribute_value"),"dd.MM.yyyy"));
            break;
        default:
            attrValue = QVariant(QString(command->parameter("attribute_value")));
        }
    }

    if(!target->setProperty(command->parameter("attribute_name").toLatin1().data(), attrValue)){
        errorString.append(command->parameter("attribute_name") + QString(" attribute was not set correctly. "));
    }
}


