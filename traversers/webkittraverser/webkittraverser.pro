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
TARGET = webkittraverse
CONFIG += plugin

include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/traversers

symbian: {
    TARGET.CAPABILITY=CAP_GENERAL_DLL
	TARGET.EPOCALLOWDLLDATA = 1
	TARGET.UID3 = 0x2003A9EA

	webkit_plugin.sources = webkittraverse.dll
	webkit_plugin.path = /resource/qt/plugins/traversers
 
	DEPLOYMENT += webkit_plugin
}


DEPENDPATH += . 
INCLUDEPATH += . ../../tascore/corelib

# Input
HEADERS += webkittraverse.h 
SOURCES += webkittraverse.cpp 

QT += xml webkit

DESTDIR = lib

LIBS += -L../../tascore/lib/ -lqttestability

INSTALLS += target
