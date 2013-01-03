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

symbian: {
	 TARGET.EPOCALLOWDLLDATA = 1
	 TARGET.CAPABILITY=CAP_GENERAL_DLL
	 TARGET.UID3 = 0x2003A9F2
	 dialog_fixture_plugin.sources = dialogfixture.dll
	 dialog_fixture_plugin.path = /resource/qt/plugins/tasfixtures
	 DEPLOYMENT += dialog_fixture_plugin
}
DEPENDPATH += .
INCLUDEPATH += . \
    ../../tascore/corelib
HEADERS += dialogfixture.h
SOURCES += dialogfixture.cpp

symbian:SOURCES += dialogfixture_symbian.cpp
else {
    unix:!symbian:!macx:SOURCES += dialogfixture_unix.cpp
    else {
        win32: {
            SOURCES += dialogfixture_win32.cpp
            LIBS += -lUser32
            LIBS += -lPsapi
        }
        else {
            # TODO: create separate empty implementation, when symbian is implemented
            warning( "dialogfixture.pro: Unknown platform, using symbian source." )
            SOURCES += dialogfixture_symbian.cpp
        }
    }
}

DESTDIR = lib
INSTALLS += target
LIBS += -L../../tascore/lib/ \
    -lqttestability

QT += xml widgets
