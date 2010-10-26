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
#include <QFile>
#include "tascommandparser.h"
#include "taslogger.h"


/*!
    \class TasCommandParser
    \brief TasCommandParser parses xml in tas command format and returns an object model of it.     
    
    Test framework sends ui commands to SUTs in an xml format. TasCommandParser component reads 
    and parses the xml to an object model. The object model is than passed on to command handlers.
    
    The format is very simple. Below is an example from mouse click command on a GraphicsItem type 
    object. The type attributein the target defines the type of the object. QGraphicsItem type object
    are of type "graphicsView" and normal QWidgets are of type "standardView".
    
    <TasCommands id="4320" service="uiState" async="true">
        <Target TasId="102859040" type="graphicsView">
            <object objectName="" className="" text=>"" ..>
               <object objectName="" className="">
                  <searchParams/>
               </object>
            </object>
            <Command name="MouseClick" button="1" delay="0">
            </Command>
        </Target>
    </TasCommands>           
           
    The server details are used to send the responses (ui state) back to the test framework using 
    tcp communication. The normal scenario is that the commanding ui does not results in 
    ui states being send and the test framework asks for the separately.
    
     
*/


/*!

   Returns a TasCommandModel containing all of the details from the command xml.
   In cases where the xml cannot be parsed a null is returned so always check the 
   model before using it.

*/
TasCommandModel* TasCommandParser::parseCommandXml(const QString& commandXml)
{    
    TasCommandModel* model = 0;        
    QDomDocument doc("TasCommands");    
    QString errorMsg;
    if (doc.setContent(commandXml, &errorMsg)){        
        model = new TasCommandModel();
        model->setSourceString(commandXml);
        QDomElement root = doc.documentElement();
        model->setId(root.attribute("id"));
        model->setUId(root.attribute("applicationUid"));
        model->setName(root.attribute("name"));
        model->setService(root.attribute("service"));        
        if(root.attribute("async") == "true"){
            model->setAsynchronous(true);
        }
        if(!root.attribute("interval").isEmpty()){
            model->setInterval(root.attribute("interval").toInt());
        }
                  
        if(root.attribute("multitouch") == "true"){
            model->setMultitouch(true);
        }

        //get targets
        QDomNodeList targets = doc.elementsByTagName (QString("Target"));
        int count = targets.count();        
        for (int i = 0; i < count; i++){
            QDomElement target = targets.item(i).toElement();
            QString targetId = target.attribute("TasId");
            QString targetType = target.attribute("type");
            TasTarget& targetData = model->addTarget(targetId);
            targetData.setType(targetType);

            //get search params for target object(s)
            targetData.setTasTargetObject(parseTargetDetails(target));

            //get commands for target
            QDomNodeList commands = target.elementsByTagName(QString("Command"));
            int commandCount = commands.count();
            for(int j = 0 ; j < commandCount; j++){
                TasCommand& commandData = targetData.addCommand();
                QDomElement command = commands.item(j).toElement();
                QDomNamedNodeMap attributes = command.attributes();
                int attributeCount = attributes.count();
                for(int k = 0 ; k < attributeCount; k++){
                    QDomNode node = attributes.item(k);
                    QString name = node.nodeName();
                    QString value = node.nodeValue();
                    if(name == "name"){
                        commandData.setName(value);
                    }
                    else{
                        commandData.addParameter(name, value);
                    }
                }
                QDomNodeList apiParams = target.elementsByTagName(QString("param"));
                int apiParamCount = apiParams.count();
                for(int k = 0 ; k < apiParamCount; k++){
                    QDomElement apiParam = apiParams.item(k).toElement();
                    QString paramName = apiParam.attribute("name");
                    QString paramValue = apiParam.attribute("value");
                    QString paramType = apiParam.attribute("type");
                    commandData.addApiParameter(paramName, paramValue,paramType);
                }

                commandData.setText(command.text());
            }
        }        
    }
    else{
        TasLogger::logger()->error("TasCommandParser::parseCommandXml Could not parse the xml. Reason: " + errorMsg);
        //TasLogger::logger()->error("[" + commandXml + "]");
    }    
    doc.clear();
    return model;
}

TasTargetObject* TasCommandParser::parseTargetDetails(QDomElement root)
{
    TasLogger::logger()->debug("TasCommandParser::parseTargetDetails");
    TasTargetObject* obj = 0;    
    QDomElement objectDetails = root.firstChildElement(QString("object"));
    if(!objectDetails.isNull()){
        TasLogger::logger()->debug("TasCommandParser::parseTargetDetails details found");
        obj = new TasTargetObject();
        QDomNamedNodeMap attributes = objectDetails.attributes();
        int attributeCount = attributes.count();
        for(int i = 0 ; i < attributeCount; i++){
            QDomNode node = attributes.item(i);
            obj->addSearchParameter(node.nodeName(), node.nodeValue());
        } 
        obj->setChild(parseTargetDetails(objectDetails));
    }
    return obj;
}
