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


QT       += network

QT       -= gui

TARGET = cucumber_wireprotocol
DESTDIR = lib
TEMPLATE = lib

DEFINES += CUCUMBER_WIREPROTOCOL_LIBRARY

SOURCES += cucumberwireprotocolserver.cpp \
    cucumberstepdata.cpp

HEADERS += cucumberwireprotocolserver.h\
        cucumber_wireprotocol_global.h \
    cucumberstepdata.h

include(../tasbase.pri)
include(../3rdparty/qjson.pri)

symbian {
    #Symbian specific definitions, created by QtCreator
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEA7D35B8
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = cucumber_wireprotocol.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}
