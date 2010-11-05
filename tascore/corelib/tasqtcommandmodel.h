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

class TAS_EXPORT TasCommand 
{
public:
    TasCommand(QElement& element);
    ~TasCommand();
    
public:    
    QString name() const;    
    QString parameter(const QString& name);
    QString text() const;	
    QString apiParameter(const QString& name);
	QElement& documentElement();

private:    
	QDomElement& mElement;
};

class TAS_EXPORT TasTargetObject
{
public:  
    TasTargetObject(QDomElement& element);
	~TasTargetObject();

public:
	QString objectName() const;
	QString className() const;
	QHash<QString,QString> searchParameters() const;
	void setChild(TasTargetObject* child);
	TasTargetObject* child() const;
	QElement& documentElement();

private:
	TasTargetObject* mChild;
	QDomElement& mElement;
};


class TAS_EXPORT TasTarget 
{
public:
    TasTarget(QDomElement element);
    ~TasTarget();
    
public:    
    QList<TasCommand*> commandList() const;
    //TasCommand& addCommand(const QString& name=0);    
    QString id() const;
    QString type() const;
	TasCommand* findCommand(const QString& commandName);
	TasTargetObject* targetObject() const;
	//	void setTasTargetObject(TasTargetObject* object);
	QElement& documentElement();

private:
    QList<TasCommand*> mCommands;
	TasTargetObject* mTargetObject;  
	QDomElement mElement;
};

class TAS_EXPORT TasCommandModel
{    
public:
    ~TasCommandModel();

private:
    TasCommandModel();
    
public:
	static TasCommandModel* makeModel(const QString& sourceXml);

    QList<TasTarget*> targetList();

    QString id() const;
    QString uid() const;
    QString name() const;
    QString service() const;

	TasTarget* findTarget(const QString& id);
	QString sourceString() const;
	
	bool isAsynchronous();
	int interval();
	bool isMultitouch();

private:
    QList<TasTarget*> mTargets;
	QDomDocument mDocument;
	QDomElement mElement;
};

#endif
