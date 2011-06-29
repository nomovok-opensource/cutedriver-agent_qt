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
TARGET = mobilitysysinfofixture
CONFIG += plugin
CONFIG += mobility

MOBILITY += systeminfo

include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/tasfixtures

symbian: {
	TARGET.EPOCALLOWDLLDATA = 1
        TARGET.CAPABILITY = CAP_GENERAL_DLL LocalServices NetworkServices ReadUserData UserEnvironment Location  ReadDeviceData
        TARGET.UID3 = 0x2003A9F7


        mobilitysysinfo_fixture_plugin.sources = mobilitysysinfofixture.dll
        mobilitysysinfo_fixture_plugin.path = /resource/qt/plugins/tasfixtures
 
        DEPLOYMENT += mobilitysysinfo_fixture_plugin
	
}


DEPENDPATH += . 
INCLUDEPATH += . ../../tascore/corelib

# Input
HEADERS += mobilitysysinfofixture.h
SOURCES += mobilitysysinfofixture.cpp

DESTDIR = lib

INSTALLS += target

LIBS += -L../../tascore/lib/ -lqttestability
QT += xml
