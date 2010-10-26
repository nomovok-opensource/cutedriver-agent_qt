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
#include "tasconstants.h"

class TasCommand;
class TasTarget;
class TasCommandModel;

class TAS_EXPORT TasCommand 
{
public:
    TasCommand(const QString& name=0);
    TasCommand(const TasCommand& command);
    ~TasCommand();
    
public:    
    void setName(const QString& name);
    QString name() const;    
    void addParameter(const QString& name, const QString& value);
    QString parameter(const QString& name);
    void setText(const QString& text);
    QString text() const;	
	void addApiParameter(const QString& name, const QString& value, const QString& type=QVariant::typeToName(QVariant::String));
    QString apiParameter(const QString& name);
	QPair<QString,QString> apiParameterAndType(const QString& name);
	QHash<QString, QString> getApiParameters() const;
	QHash<QString, QPair<QString,QString> > getApiParametersAndTypes() const;

protected:
	QHash<QString, QString> getParameters() const;


private:    
    QString commandName;
    QString valueText;
    QHash<QString, QString> parameters;
    QHash<QString, QPair<QString,QString> > apiParameters;
};

class TAS_EXPORT TasTargetObject
{
public:  
    TasTargetObject();
    TasTargetObject(const TasTargetObject& object);
	~TasTargetObject();

public:
	QString objectName() const;
	QString className() const;
	QHash<QString,QString> searchParameters() const;
	void setObjectName(const QString name);
	void setClassName(const QString className);
	void addSearchParameter(QString name, QString value);
	void setChild(TasTargetObject* child);
	TasTargetObject* child() const;
	void setObjectId(const QString id);
	QString objectId() const;


private:
	QString mObjectName;
	QString mClassName;
	QString mObjectId;
	QHash<QString,QString> mSearchParams;
	TasTargetObject* mChild;
};


class TAS_EXPORT TasTarget 
{
public:
    TasTarget(const QString& id);
    TasTarget(const TasTarget& target);
    ~TasTarget();
    
public:    
    QList<TasCommand*> commandList() const;
    TasCommand& addCommand(const QString& name=0);    
    void setId(const QString& id);
    void setType(const QString& type);
    QString id() const;
    QString type() const;
	TasCommand* findCommand(const QString& commandName);
	TasTargetObject* targetObject() const;
	void setTasTargetObject(TasTargetObject* object);

private:
    QList<TasCommand*> mCommands;
    QString mTargetId;
    QString mTargetType;      
	TasTargetObject* mTargetObject;  
};

class TAS_EXPORT TasCommandModel
{    
public:
    TasCommandModel();
    ~TasCommandModel();
    
public:
    QList<TasTarget*> targetList();
    TasTarget& addTarget(const QString& id);
    void setId(const QString& id);
    QString id() const;
    void setUId(const QString& uid);
    QString uid() const;
    void setName(const QString& name);
    QString name() const;
    void setService(const QString& service);
    QString service() const;

	TasTarget* findTarget(const QString& id);

	void setSourceString(const QString& sourceXml);
	QString sourceString() const;
	
	void setAsynchronous(bool asynchronous);
	bool isAsynchronous();
	void forceUiUpdate(bool force);
	bool forceUiUpdate();
	
	void setInterval(int interval);
	int interval();

	bool isMultitouch();
	void setMultitouch(bool multitouch);

private:
    QList<TasTarget*> mTargets;
    QString mModelId;    
    QString mModelUid;    
    QString mModelName;    
    QString mModelService;    
	QString mSourceString;
	bool mAsynchronous;
	bool mForceUiUpdate;
	bool mMultitouch;
	int mInterval;
};

#endif
