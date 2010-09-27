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

CONFIG(maemo){
LIBS += -lqmsystem
DEFINES += TAS_MAEMO
}

# Input
include(corelib/uilib.pri)
include(corelib/corelib.pri)
include(services/services.pri)

HEADERS += $$PUBLIC_HEADERS

symbian: {	
    TARGET.CAPABILITY=CAP_GENERAL_DLL
    TARGET.EPOCALLOWDLLDATA = 1

    BLD_INF_RULES.prj_exports += "conf/qt_testability.ini /epoc32/winscw/c/qt_testability/qt_testability.ini"
	!CONFIG(no_mobility){
	  MOBILITY += systeminfo
      CONFIG += mobility
	}
	#in some envs these are not included by default
	LIBS += -lcone 
    LIBS += -leikcore 
    LIBS += -lhal
#if ( NCP_COMMON_S60_VERSION_SUPPORT >= S60_VERSION_50 && NCP_COMMON_FAMILY_ID >= 70 )
	LIBS += -llibegl
#endif

    for(PUBLIC_HEADER, PUBLIC_HEADERS) {
        contains(QMAKE_HOST.os, Windows):PUBLIC_HEADER = $$section(PUBLIC_HEADER, ":", 1)
        EXPORT_PATH = $$sprintf($$EXPORT_DIR, $$basename(PUBLIC_HEADER))
		BLD_INF_RULES.prj_exports *= "$$PUBLIC_HEADER $$EXPORT_PATH"
	}

}


QT += network xml testlib gui webkit

#configuration file
configuration.files = conf/qt_testability.ini
unix:{
	configuration.path = /etc/qt_testability 

  HEADERS.path = /usr/include/tdriver/
  
  HEADERS_OLD.path = /usr/include/matti/
  HEADERS_OLD.files = $$HEADERS
  INSTALLS += HEADERS_OLD


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

  HEADERS_OLD.path = /usr/include/matti/
  HEADERS_OLD.files = $$HEADERS
  INSTALLS += HEADERS_OLD
}
win32:{
	configuration.path=/qttas/conf
  HEADERS.path = /qttas/inc
}

INSTALLS += target
INSTALLS += configuration

HEADERS.files = $$HEADERS
INSTALLS += HEADERS




