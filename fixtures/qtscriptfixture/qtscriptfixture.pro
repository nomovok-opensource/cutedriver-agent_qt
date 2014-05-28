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
TARGET = qtscriptfixture
CONFIG += plugin

include(../../tasbase.pri)

QT += script

target.path = $$TAS_TARGET_PLUGIN/tasfixtures
stepsfiles.path = $$TAS_TARGET_STEPS
stepsfiles.files = qtscriptfixture.default.steps

symbian: {
        TARGET.EPOCALLOWDLLDATA = 1
        TARGET.CAPABILITY=CAP_GENERAL_DLL
        TARGET.UID3 = 0x2003AA00

        qtscript_fixture_plugin.sources = qtscriptfixture.dll
        qtscript_fixture_plugin.path = /resource/qt/plugins/tasfixtures
        DEPLOYMENT += qtscript_fixture_plugin
}

DEPENDPATH += .
INCLUDEPATH += . ../../tascore/corelib

HEADERS += qtscriptfixture.h
SOURCES += qtscriptfixture.cpp

DESTDIR = lib

INSTALLS += target
INSTALLS += stepsfiles

LIBS += -L../../tascore/lib/ -lqttestability
