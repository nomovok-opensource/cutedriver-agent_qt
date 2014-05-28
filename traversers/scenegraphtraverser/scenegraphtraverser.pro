############################################################################
## 
## Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
## All rights reserved. 
## Contact: Nokia Corporation (testabilitydriver@nokia.com) 
## 
## This file is part of Testability Driver Qt Agent
## 
## If you have questions regarding the use of this file, please contact 
## Nokia at testabilitydriver@nokia.com . 
## 
## This library is free software; you can redistribute it and/or 
## modify it under the terms of the GNU Lesser General Public 
## License version 2.1 as published by the Free Software Foundation 
## and appearing in the file LICENSE.LGPL included in the packaging 
## of this file. 
## 
############################################################################



TEMPLATE = lib
TARGET = scenegraphtraverse
CONFIG += plugin

include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/traversers

symbian: {
  TARGET.CAPABILITY=CAP_GENERAL_DLL
  TARGET.EPOCALLOWDLLDATA = 1
	TARGET.UID3 = 0x2003A9ED

    scenegraphtraverse_plugin.sources = qttraverse.dll
    scenegraphtraverse_plugin.path = /resource/qt/plugins/traversers
 
    DEPLOYMENT += scenegraphtraverse_plugin
}


DEPENDPATH += . 
INCLUDEPATH += . ../../tascore/corelib


isEmpty(QT_PRIVATE_HEADERS) {
    QT_PRIVATE_HEADERS = $$[QT_INSTALL_HEADERS]
}

message($${QT_PRIVATE_HEADERS})

isEmpty(QT_PRIVATE_HEADERS) {
    message(BAD! - no_private_headers_found)
    QT_PRIVATE_HEADERS = $$[QT_INSTALL_HEADERS]
} else {
    message(GOOD - private_headers_found)

    exists($${QT_PRIVATE_HEADERS}/QtQml/5.0.0/QtQml/private/qqmlmetatype_p.h) {
        message(GOOD - qtqml private headers found and used)
        INCLUDEPATH += $$quote($${QT_PRIVATE_HEADERS})/QtQml/5.0.0/QtQml
        INCLUDEPATH += $$quote($${QT_PRIVATE_HEADERS})/QtCore/5.0.0/QtCore
        INCLUDEPATH += $$quote($${QT_PRIVATE_HEADERS})/QtV8/5.0.0/QtV8
        DEFINES += USE_QTQML_PRIVATE_HEADERS
    }
}



# Input
HEADERS += scenegraphtraverse.h
SOURCES += scenegraphtraverse.cpp
OTHER_FILES += scenegraphtraverse.json

QT += xml widgets qml quick

DESTDIR = lib

LIBS += -L../../tascore/lib/ -lqttestability

INSTALLS += target
