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

TARGET = qttasserver
DESTDIR = bin
target.path = $$TAS_TARGET_BIN
DEFINES += TDTASSERVER

symbian: {
    TARGET.CAPABILITY=ALL -TCB
    TARGET.VID = VID_DEFAULT
    TARGET.EPOCALLOWDLLDATA = 1 
    TARGET.EPOCHEAPSIZE = 0x20000 0xD00000
    TARGET.UID3 = 0x2003A9E1

    LIBS += -leikcore 
    LIBS += -leuser
    LIBS += -lws32
    LIBS += -lapgrfx
    LIBS += -lmemspydriverclient
    LIBS += -lhal
    LIBS += -lcone
    LIBS += -lcfclient -lcfservices
    INCLUDEPATH += /epoc32/include/platform/memspy/driver /epoc32/include/platform/mw
#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )
	LIBS += -llibEGL
#endif
}

win32: {
	LIBS += -lUser32
    LIBS +=	-lKernel32
}

INCLUDEPATH += .
INCLUDEPATH += services corelib
INCLUDEPATH += ../tascore/corelib

DEPENDPATH += . inc src services corelib

# Input
SOURCES += main.cpp

include(corelib/corelib.pri)
include(services/services.pri)
include(../tascore/corelib/corelib.pri)

HEADERS += $$PUBLIC_HEADERS

QT -= gui
QT += network xml 

unix: {
	QT += testlib
}

CONFIG(maemo){
DEFINES += TAS_MAEMO
}

INSTALLS += target



