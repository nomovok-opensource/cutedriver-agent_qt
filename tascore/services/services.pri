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

HEADERS += $$PWD/uistateservice.h 
HEADERS += $$PWD/closeappservice.h 
HEADERS += $$PWD/fixtureservice.h 
HEADERS += $$PWD/screenshotservice.h 
#HEADERS += $$PWD/platformservice.h
HEADERS += $$PWD/eventservice.h 
HEADERS += $$PWD/objectservice.h 
HEADERS += $$PWD/uicommandservice.h 
HEADERS += $$PWD/webkitcommandservice.h 
HEADERS += $$PWD/recorderservice.h 
HEADERS += $$PWD/confservice.h 
HEADERS += $$PWD/infoservice.h 

SOURCES += $$PWD/uistateservice.cpp 
SOURCES += $$PWD/closeappservice.cpp 
SOURCES += $$PWD/fixtureservice.cpp 
SOURCES += $$PWD/screenshotservice.cpp 
#SOURCES += $$PWD/platformservice.cpp
SOURCES += $$PWD/eventservice.cpp 
SOURCES += $$PWD/objectservice.cpp 
SOURCES += $$PWD/uicommandservice.cpp 
SOURCES += $$PWD/webkitcommandservice.cpp 
SOURCES += $$PWD/recorderservice.cpp 
SOURCES += $$PWD/confservice.cpp 
SOURCES += $$PWD/infoservice.cpp 

include ($$PWD/interactionhandlers/handlers.pri)

 
