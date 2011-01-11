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

TARGET = qttas_ui
DESTDIR = bin
DEFINES += TDTASSERVER

target.path = $$TAS_TARGET_BIN

symbian: {
    TARGET.UID3 = 0xA89FD3E0
    TARGET.CAPABILITY=ALL -TCB
	TARGET.VID = VID_DEFAULT
  	TARGET.EPOCALLOWDLLDATA = 1 
	TARGET.EPOCHEAPSIZE = 0x20000 0x1400000
    LIBS += -lhal
    INCLUDEPATH += /epoc32/include/platform/memspy/driver
#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )
	LIBS += -llibEGL
#endif
}



INCLUDEPATH += . src
INCLUDEPATH += ../tascore/corelib

DEPENDPATH += . src

# Input
HEADERS += servermonitor.h
HEADERS += serverwindow.h
SOURCES += main.cpp
SOURCES += servermonitor.cpp
SOURCES += serverwindow.cpp

include(../tascore/corelib/corelib.pri)
HEADERS += $$PUBLIC_HEADERS


QT += network xml 
INSTALLS += target

unix:!symbian:!macx:!CONFIG(no_x) {
  LIBS += -lX11 -lXtst 
}


