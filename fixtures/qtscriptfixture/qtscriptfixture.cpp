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
 


#include <QGraphicsWidget>
#include <QMouseEvent>

#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>

#include "qtscriptfixture.h"
#include "testabilityutils.h"
#include "taslogger.h"

Q_EXPORT_PLUGIN2(qtscriptfixture, QtScriptFixturePlugin)

/*!
  \class QtScriptFixturePlugin
  \brief 
*/

QtScriptFixturePlugin::QtScriptFixturePlugin(QObject* parent) :QObject(parent) {}
QtScriptFixturePlugin::~QtScriptFixturePlugin() {}

/*
QVariantMap testMap;
testMap.insert("number", 1);
testMap.insert("foo", "bar");

QVariantList testList;
testList << 1 << "foobar" << testMap;
TasLogger::logger()->debug("testDumpQVariant: " + dumpQVariant(testList));
*/

static inline QString dumpQVariant(const QVariant &var) {
    QStringList result;
    const char *surround = "??";
    if(var.canConvert(QVariant::Map)) {
        const QVariantMap varMap(var.toMap());
        surround = "{}";
        foreach(const QString &key, varMap.keys()) {
            result << '"' + key + "\":" + dumpQVariant(varMap[key]);
        }
    }
    else if (var.canConvert(QVariant::List)) {
        surround = "[]";
        foreach(const QVariant &item, var.toList()) {
            result << dumpQVariant(item);
        }
    }
    else {
        surround = "()";
        result << '"' + var.toString() + '"';
    }
    return surround[0] + result.join(",") + surround[1];
}


static inline QString dumpQScriptValue(const QScriptValue &value)
{
    QStringList result;
    const char *surround = "<>";
    QScriptValueIterator it(value);
    int count = 0;
    while (it.hasNext()) {
        it.next();
        ++count;
        result << "\"" + it.name() + "\":" + dumpQScriptValue(it.value());
    }

    if (0 == count) {
        surround = "()";
        result << '"' + value.toString() + '"';
    }

    return surround[0] + result.join(",") + surround[1];
}


static inline QString checkScript(QScriptEngine &engine, const QString &script) {

    QScriptSyntaxCheckResult result(engine.checkSyntax(script));
    switch(result.state()) {

    case QScriptSyntaxCheckResult::Valid:
        return "";

    case QScriptSyntaxCheckResult::Intermediate:
        return "Script error: Incomplete script";

    default:
        return QString("Script error: %1:%2:%3")
                .arg(result.errorLineNumber())
                .arg(result.errorColumnNumber())
                .arg(result.errorMessage());
    }
}


bool QtScriptFixturePlugin::execute(
        void* objectInstance, 
        QString actionName, 
        QHash<QString, QString> parameters, 
        QString& stdOut)
{

    Q_UNUSED(stdOut);

    TasLogger::logger()->debug("> QtScriptFixturePlugin::execute");

    QString objType = parameters.value(OBJECT_TYPE);
    QObject *targetObj = NULL;

    if (objType == APPLICATION_TYPE) {
        targetObj = TestabilityUtils::getApplicationWindow();
        // objectInstance ignored
    }
    else if (objType == WIDGET_TYPE) {
        targetObj = qobject_cast<QObject*>(reinterpret_cast<QWidget*>(objectInstance));
    }
    else if (objType == GRAPHICS_ITEM_TYPE) {
        QGraphicsItem *item = reinterpret_cast<QGraphicsItem*>(objectInstance);
        targetObj = qobject_cast<QObject*>(item->toGraphicsObject());
        if (!targetObj) {
            stdOut = "QGraphicsItem was not QGraphicsObject";
            /*
            targetObj = qobject_cast<QObject*>(item->scene());
            if (!targetObj) {
                stdOut = "could not generate QObject from target QGraphicsItem";
            }
            */
        }
    }

    if (!targetObj){
        TasLogger::logger()->warning("QtScriptFixturePlugin::execute invalid target object");
        return false;
    }

    //    TasLogger::logger()->debug(QString(">> ") + targetObj->metaObject()->className() + " obj " + targetObj->objectName());
    //    foreach (QObject* child, targetObj->children()) {
    //        TasLogger::logger()->debug(QString(">>>>> ") + child->metaObject()->className() + " obj " + child->objectName());
    //    }

    bool ret = false;

    QScriptEngine engine;

#if 1
    QScriptValue contextObjectValue = engine.newQObject(targetObj);
    engine.pushContext()->setThisObject(contextObjectValue);
#else // not really needed, when "this" is target object
    {
        QScriptValue targetValue = engine.newObject();
        targetValue.setProperty("target", engine.newQObject(targetObj));
        targetValue.setProperty("action", QScriptValue("actionName"));
        TasLogger::logger()->debug("targetValue: " +  dumpQScriptValue(targetValue));
        engine.globalObject().setProperty("target", targetValue);
    }
    TasLogger::logger()->debug("target property: " +  dumpQScriptValue(engine.globalObject().property("target")));
#endif

    {
        QScriptValue tmpVal = engine.newObject();
        foreach(const QString &key, parameters.keys()) {
            TasLogger::logger()->debug("param: " +  key + "=" + parameters[key]);
            tmpVal.setProperty(key, parameters[key]);
        }
        engine.globalObject().setProperty("tdriver", tmpVal);
    }
    TasLogger::logger()->debug("tdriver property: " +  dumpQScriptValue(engine.globalObject().property("tdriver")));

    if (actionName == "eval") {
        QString script = parameters.value("script");

        stdOut = checkScript(engine, script);
        if (stdOut.isEmpty()) {
            QScriptValue result = engine.evaluate(script);
            if (engine.hasUncaughtException()) {
                stdOut = engine.uncaughtException().toString();
            }
            else {
                stdOut = result.toString();
                ret = true;
            }
        }
    }

    else if (actionName == CUCUMBER_STEP_ACTION) {
        const QString regExp = parameters.value("regExp");
        TasLogger::logger()->debug("QtScriptFixturePlugin::execute cucumber step " + regExp);

        static QMap<QString, QString> regExpScripts;
        if (regExpScripts.isEmpty()) {
            regExpScripts["I call (\\S+) on application object (\\S+)"]
                    = "eval('this.'+args[1]+'.'+args[0]+'()')";
            regExpScripts["application object (\\S+) has (\\S+) ['\"](\\S+)['\"]"]
                    = "if (args[2] != eval('this.'+args[0]+'.'+args[1]).toString()) { throw 'No match!'; } ";
        }

        if (!regExpScripts.contains(regExp)) {
            stdOut = "no match for step '" + regExp + "'";
        }
        else {
            QScriptValue argsValue = engine.evaluate("JSON.parse(tdriver.qjson)");
            if (engine.hasUncaughtException()) {
                stdOut = QString("JSON error:") + engine.uncaughtException().toString();
            }
            else {
                engine.globalObject().setProperty("args", argsValue);
                TasLogger::logger()->debug("argsValue: " + dumpQScriptValue(argsValue));
                TasLogger::logger()->debug("args property: " + dumpQScriptValue(engine.globalObject().property("args")));

                QScriptValue scriptReturnValue = engine.evaluate(regExpScripts[regExp]);
                if (engine.hasUncaughtException()) {
                    stdOut = "QtScript code '" + regExpScripts[regExp] + "': " + engine.uncaughtException().toString();
                }
                else {
                    TasLogger::logger()->debug("> QtScriptFixturePlugin::execute evaluate result:" + scriptReturnValue.toString());
                    stdOut = scriptReturnValue.toString();
                    ret = true;
                }
            }
        }
    }

    else if (actionName == "test") {
        stdOut = "test successful";
        ret = true;
    }

    return ret;
}
