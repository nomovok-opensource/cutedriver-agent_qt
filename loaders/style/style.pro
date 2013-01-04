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
TARGET = testabilitystyle
include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/styles
CONFIG += plugin

symbian: {
    TARGET.CAPABILITY=CAP_GENERAL_DLL
	TARGET.EPOCALLOWDLLDATA = 1
	TARGET.UID3 = 0x2003A9E5

	
	testabilitystyle_plugin.sources = testabilitystyle.dll
	testabilitystyle_plugin.path = /resource/qt/plugins/styles
 
	DEPLOYMENT += testabilitystyle_plugin
}
DESTDIR = lib

DEPENDPATH += . 

INCLUDEPATH += . ../../tascore/corelib

HEADERS += testability.h  
SOURCES += testability.cpp
OTHER_FILES += testabilitystyle.json

INSTALLS += target

QT += widgets



