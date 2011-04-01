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
        
#include "tasxmlwriter.h"

TasXmlWriter::TasXmlWriter(QTextStream& out)
    :mXmlStream(out)
{
    mXmlStream.setCodec("UTF-8");
}

TasXmlWriter::~TasXmlWriter()
{}

void TasXmlWriter::openElement(const QString& name)
{
    mXmlStream << "<" << name << ">";
}

void TasXmlWriter::openElement(const QString& name, QMap<QString,QString> attributes)
{
    mXmlStream << "<" << name << " ";

    QMap<QString, QString>::const_iterator i;

    for (i = attributes.constBegin(); i != attributes.constEnd(); ++i)
        mXmlStream << i.key() << "=\"" << i.value() << "\" ";
        
    mXmlStream << ">";
}

void TasXmlWriter::addTextContent(const QString& content)
{
    mXmlStream << content;
}

void TasXmlWriter::closeElement(const QString& name)
{
    mXmlStream << "</" << name << ">";
}
