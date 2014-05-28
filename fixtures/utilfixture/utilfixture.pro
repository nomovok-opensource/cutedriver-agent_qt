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
TARGET = utilfixture
CONFIG += plugin
QT += xml testlib network sql
include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/tasfixtures

symbian: {
	TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY=CAP_GENERAL_DLL
    TARGET.UID3 = 0x2003A9FD

	util_fixture_plugin.sources = utilfixture.dll
	util_fixture_plugin.path = /resource/qt/plugins/tasfixtures
 
	DEPLOYMENT += util_fixture_plugin
	
}

ROOT_DIR = $$PWD/../../../../..
CWRT_DIR = $$ROOT_DIR/cwrt
WRT_DIR = $$CWRT_DIR/wrt

include($$WRT_DIR/cwrt-webkit.pri)

DEPENDPATH += . 
INCLUDEPATH += . ../../tascore/corelib \
               	$$WRT_DIR/widgetmanager/src \
               	$$WRT_DIR/tests/helpers \
               	$$WRT_DIR/serviceproviders/appmgt/appmgtsecregistryclient/platform/s60/client/inc \
               	$$WRT_DIR/widgetmanager/src/ \
               	$$APP_LAYER_SYSTEMINCLUDE \
                $$PWD/platform/symbian/ \
                $$MW_LAYER_SYSTEMINCLUDE \
                $$ROOT_DIR/epoc32/include/mw/cwrt \

# Input
HEADERS += utilfixtureplugin.h \
           $$WRT_DIR/tests/helpers/testhelpers.h  \
           $$WRT_DIR/widgetmanager/src/WebAppRegistry.h \
           $$WRT_DIR/widgetmanager/src/W3CXmlParser/w3csettingskeys.h \
           
contains(what, tenone){
 HEADERS +=  $$ROOT_DIR/epoc32/include/mw/usif/scr/scr.h \
}

SOURCES += utilfixtureplugin.cpp \


DESTDIR = lib

INSTALLS += target

LIBS += -L../../tascore/lib/ \
		-lqttestability \
		-lWidgetUtils \


contains (what, ninetwo){
 LIBS +=  -lwidgetregistryclient \
}
contains (what, tenone){
     DEFINES += QTWRT_USE_USIF
     #INCLUDEPATH +=  $$PWD/../../../src $$PWD/../../../src/W3CXmlParser
     LIBS += -lscrclient \
             -lsecwidgetregistryclient \
}
