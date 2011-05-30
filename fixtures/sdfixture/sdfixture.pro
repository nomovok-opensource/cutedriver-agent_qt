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
TARGET = sdfixture
CONFIG += plugin

include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/tasfixtures

symbian: {
	TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY=CAP_GENERAL_DLL
    TARGET.UID3 = 0x2003A9F9

	sd_fixture_plugin.sources = sdfixture.dll
	sd_fixture_plugin.path = /resource/qt/plugins/sdfixtures
 
	DEPLOYMENT += sd_fixture_plugin
	LIBS += -lecom
	LIBS += -lxqutils
}

CONFIG += mobility


DEPENDPATH += . 
INCLUDEPATH += . 

# Input
HEADERS += sdfixture.h
SOURCES += sdfixture.cpp

DESTDIR = lib

INSTALLS += target
