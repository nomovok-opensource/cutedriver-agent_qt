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
TARGET = testability
include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/testability
CONFIG += plugin

symbian: {
    TARGET.CAPABILITY=CAP_GENERAL_DLL
	TARGET.EPOCALLOWDLLDATA = 1
	TARGET.UID3 = 0x2003A9E6

	
	testability_plugin.sources = testability.dll
	testability_plugin.path = /resource/qt/plugins/testability
 
	DEPLOYMENT += testability_plugin
     LIBS += -lcentralrepository 
}
DESTDIR = lib

DEPENDPATH += . 

INCLUDEPATH += . ../../tascore/corelib

HEADERS += testability.h  
SOURCES += testability.cpp
OTHER_FILES += testability.json

INSTALLS += target

QT += widgets


