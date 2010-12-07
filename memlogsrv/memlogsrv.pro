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


TEMPLATE = app

mac {
   CONFIG -= app_bundle
}


include(../tasbase.pri)

TARGET = qttasmemlog_srv
DESTDIR = bin
target.path = $$TAS_TARGET_BIN
DEFINES += TDTASSERVER

symbian: {
    TARGET.CAPABILITY = ReadUserData WriteUserData ReadDeviceData WriteDeviceData SwEvent PowerMgmt
	TARGET.VID = VID_DEFAULT
  	TARGET.EPOCALLOWDLLDATA = 1 
	TARGET.EPOCHEAPSIZE = 0x20000 0x1400000
    LIBS += -lMemSpyDriverClient
    INCLUDEPATH += /epoc32/include/platform/memspy/driver

    LIBS += -lhal
    INCLUDEPATH += /epoc32/include/platform/memspy/driver
#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )
	LIBS += -llibegl
#endif
}

win32: {
	LIBS +=  -lUser32
}

INCLUDEPATH += .
INCLUDEPATH += services corelib
INCLUDEPATH += ../tascore/corelib
DEPENDPATH += . services corelib

include(../tascore/corelib/corelib.pri)
HEADERS += $$PUBLIC_HEADERS

# Input
SOURCES += main.cpp

include(corelib/corelib.pri)
include(services/services.pri)

QT -= gui
QT += network xml 
INSTALLS += target

unix:!symbian:!macx:!CONFIG(no_x) {
  LIBS += -lX11 -lXtst
}


