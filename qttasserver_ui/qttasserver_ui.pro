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
target.path = $$TAS_TARGET_BIN

symbian: {
    TARGET.CAPABILITY=ALL -TCB
	TARGET.VID = VID_DEFAULT
  	TARGET.EPOCALLOWDLLDATA = 1 
	TARGET.EPOCHEAPSIZE = 0x20000 0x1400000
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

QT += network xml 
INSTALLS += target
 
LIBS += -L../tascore/lib/ -lqttestability



unix:!symbian:!macx {
  LIBS += -lX11 -lXtst 
}


