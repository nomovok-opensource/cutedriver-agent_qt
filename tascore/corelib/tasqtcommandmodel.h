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

#ifndef TASTCOMMANDMODEL_H
#define TASTCOMMANDMODEL_H

#include <QList>
#include <QHash>
#include <QPair>
#include <QVariant>
#include <QDomElement>
#include <QDomDocument>

#include "tasconstants.h"

class TasCommand;
class TasTarget;
class TasCommandModel;

class TasDomObject
{
public:
    void addDomAttribute(const QString& name, const QString& value);
	void setText(const QString& text);
    QDomElement domElement() const;
    QString parameter(const QString& name);

protected:
    QDomElement addChild(const QString& name);

    QDomElement mElement;
};

class TAS_EXPORT TasCommand : public TasDomObject
{
public:
    TasCommand(QDomElement element);
    TasCommand(const TasCommand& other);
    ~TasCommand();
    
public:    
    QString name() const;    
    QString text() const;	
    QString apiParameter(const QString& name);
	QHash<QString, QString> getApiParameters() const;
    QString parameter(const QString& name);

	void addApiParameter(const QString& name, const QString& value, const QString& type);
};

class TAS_EXPORT TasTargetObject : public TasDomObject
{
public:  
    TasTargetObject(QDomElement element);
	~TasTargetObject();

public:
	QString objectName() const;
	QString className() const;
	QString objectId() const;
	QHash<QString,QString> searchParameters() const;
	void setChild(TasTargetObject* child);
	TasTargetObject* child() const;

private:
	TasTargetObject* mChild;
};


class TAS_EXPORT TasTarget : public TasDomObject
{
public:
    TasTarget(QDomElement element);
    TasTarget(const TasTarget& other);
    ~TasTarget();
    
public:    
    QList<TasCommand*> commandList() const;
    QString id() const;
    QString type() const;
	TasCommand* findCommand(const QString& commandName);
	TasTargetObject* targetObject() const;

	TasCommand& addCommand();

private:
	void initialize();

private:
    QList<TasCommand*> mCommands;
	TasTargetObject* mTargetObject;  
};

class TAS_EXPORT TasCommandModel : public TasDomObject
{    
public:
    ~TasCommandModel();

private:
    TasCommandModel(QDomDocument* document);
    
public:
	static TasCommandModel* makeModel(const QString& sourceXml);
	static TasCommandModel* createModel();

    QList<TasTarget*> targetList();

    QString id() const;
    QString uid() const;
    QString name() const;
    QString service() const;

	TasTarget& addTarget();
	TasTarget* findTarget(const QString& id);
	QString sourceString(bool original=true) const;
	
	bool isAsynchronous();
	int interval();
	bool isMultitouch();
	bool onlyFragment();

        //TasCommandModel* clone();
private:
    QList<TasTarget*> mTargets;
	QDomDocument* mDocument;
	QString mSource;	
};

#endif
