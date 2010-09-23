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

HEADERS += services/uistateservice.h
HEADERS += services/closeappservice.h
HEADERS += services/fixtureservice.h
HEADERS += services/screenshotservice.h
#HEADERS += services/platformservice.h
HEADERS += services/eventservice.h
HEADERS += services/objectservice.h
HEADERS += services/uicommandservice.h
HEADERS += services/webkitcommandservice.h
HEADERS += services/recorderservice.h
HEADERS += services/confservice.h
HEADERS += services/infoservice.h

SOURCES += services/uistateservice.cpp
SOURCES += services/closeappservice.cpp
SOURCES += services/fixtureservice.cpp
SOURCES += services/screenshotservice.cpp
#SOURCES += services/platformservice.cpp
SOURCES += services/eventservice.cpp
SOURCES += services/objectservice.cpp
SOURCES += services/uicommandservice.cpp
SOURCES += services/webkitcommandservice.cpp
SOURCES += services/recorderservice.cpp
SOURCES += services/confservice.cpp
SOURCES += services/infoservice.cpp

include (./interactionhandlers/handlers.pri)
