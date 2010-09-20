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
 


#ifndef UICOMMANDSERVICE_H
#define UICOMMANDSERVICE_H

#include <QObject>
#include <QTimer>
#include <QQueue>
#include <QList>

#include "tasservicebase.h"
#include "testabilityutils.h"

struct TargetData
{
  TasCommand* command;
  QWidget* target;
  QPoint targetPoint;
  QGraphicsItem* targetItem;
};

class InteractionHandler : public TestabilityUtils
{
public:
  virtual ~InteractionHandler(){};

  virtual bool executeInteraction(TargetData data) = 0;
  virtual bool executeMultitouchInteraction(QList<TargetData> dataList){return false;}
};


class UiCommandService : public QObject, public TasServiceBase
{
  Q_OBJECT

public:
    UiCommandService(QObject* parent = 0);
	~UiCommandService();

	/*!
	  From ServiceInterface
	*/
	bool executeService(TasCommandModel& model, TasResponse& response);

	QString serviceName()const { return UI_COMMAND; }

private slots:
	void startTimer();
    void executeNextCommand();

private:
	//void performUiCommands(TasTarget* commandTarget);
	TargetData makeInteractionData(TasTarget* commandTarget);
	//void performCommand(TasCommand* command, QWidget* target, QPoint point, QGraphicsItem* item);
	void performCommand(TargetData data);
	void performMultitouchCommand(QList<TargetData> dataList);  
	void parseValidTargets(TasCommandModel& model);

private:
    QQueue<TasTarget*> mCommandQueue;
	QList<TasTarget*> mMultiTouchCommands;
    QTimer mTimer;
	QList<InteractionHandler*> mInteractionHandlers;
	
};

#endif
