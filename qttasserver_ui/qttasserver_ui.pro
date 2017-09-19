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

INCLUDEPATH += . src
INCLUDEPATH += ../tascore/corelib

DEPENDPATH += . src

# Input
HEADERS += src/servermonitor.h
HEADERS += src/serverwindow.h

SOURCES += main.cpp
SOURCES += src/servermonitor.cpp
SOURCES += src/serverwindow.cpp

include(../tascore/corelib/corelib.pri)
HEADERS += $$PUBLIC_HEADERS


QT += network xml widgets
INSTALLS += target

unix:!macx:!CONFIG(no_x):!wayland {
  LIBS += -lX11 -lXtst
}


