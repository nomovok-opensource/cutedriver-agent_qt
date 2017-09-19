# ###########################################################################
# #
# # Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
# # All rights reserved.
# # Contact: Nokia Corporation (testabilitydriver@nokia.com)
# #
# # This file is part of Testability Driver Qt Agent
# #
# # If you have questions regarding the use of this file, please contact
# # Nokia at testabilitydriver@nokia.com .
# #
# # This library is free software; you can redistribute it and/or
# # modify it under the terms of the GNU Lesser General Public
# # License version 2.1 as published by the Free Software Foundation
# # and appearing in the file LICENSE.LGPL included in the packaging
# # of this file.
# #
# ###########################################################################


TEMPLATE = lib
TARGET = dialogfixture
CONFIG += plugin
include(../../tasbase.pri)
target.path = $$TAS_TARGET_PLUGIN/tasfixtures

DEPENDPATH += .
INCLUDEPATH += . \
    ../../tascore/corelib
HEADERS += dialogfixture.h
SOURCES += dialogfixture.cpp
OTHER_FILES += dialogfixture.json

#TODO(rasjani) check if this works?
unix:SOURCES += dialogfixture_unix.cpp
win32: {
    SOURCES += dialogfixture_win32.cpp
    LIBS += -lUser32
    LIBS += -lPsapi
}

DESTDIR = lib
INSTALLS += target
LIBS += -L../../tascore/lib/ -lqttestability

QT += xml widgets
