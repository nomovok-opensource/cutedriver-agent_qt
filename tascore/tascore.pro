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
TARGET = qttestability
include(../tasbase.pri)

target.path = $$TAS_TARGET_LIB
DESTDIR = lib
DEFINES += BUILD_TAS
linux:QMAKE_DISTCLEAN += /usr/lib/libqttestability.so


DEPENDPATH += . corelib services services/interactionhandlers services/interactionhandlers/gestures services/interactionhandlers/eventgenerator
INCLUDEPATH += . corelib services services/interactionhandlers services/interactionhandlers/gestures services/interactionhandlers/eventgenerator

CONFIG(qml_id){
    DEFINES += QML_ID
}

CONFIG(no_webkit) {
    DEFINES += NO_WEBKIT
}

CONFIG(debug_enabled) {
DEFINES += DEBUG_ENABLED
}

# Input
include(corelib/uilib.pri)
include(corelib/corelib.pri)
include(services/services.pri)

HEADERS += $$PUBLIC_HEADERS

QT += network xml testlib gui widgets quick

!CONFIG(no_webkit) {
    QT += webkitwidgets
}


#configuration file
configuration.files = conf/qt_testability.ini
unix:!macx{
  configuration.path = /etc/qt_testability

  HEADERS.path = /usr/include/tdriver/

  # include desktop file into rpm
  CONFIG(RPM) {
    DESKTOP.files = conf/xdg/autostart/qttasserver.desktop
    DESKTOP.path = /etc/xdg/autostart/
    INSTALLS += DESKTOP
  }
}

macx: {
  configuration.path = /etc/qt_testability
  HEADERS.path = /usr/include/tdriver/
}
win32:{
  configuration.path=/qttas/conf
  HEADERS.path = /qttas/inc
}

INSTALLS += target
INSTALLS += configuration

HEADERS.files = $$HEADERS
INSTALLS += HEADERS




