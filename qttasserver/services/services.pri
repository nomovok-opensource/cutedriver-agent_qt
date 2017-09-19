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

HEADERS += services/startappservice.h
HEADERS += services/closeappservice.h
HEADERS += services/uistateservice.h
HEADERS += services/fixtureservice.h
HEADERS += services/registerservice.h
HEADERS += services/shellcommandservice.h
HEADERS += services/killservice.h
HEADERS += services/listappsservice.h
HEADERS += services/confservice.h
HEADERS += services/uicommandservice.h
HEADERS += services/foregroundservice.h
HEADERS += services/systeminfoservice.h
HEADERS += services/resourceloggingservice.h
HEADERS += services/shelltask.h
HEADERS += services/startedappservice.h

SOURCES += services/startappservice.cpp
SOURCES += services/closeappservice.cpp
SOURCES += services/uistateservice.cpp
SOURCES += services/fixtureservice.cpp
SOURCES += services/registerservice.cpp
SOURCES += services/shellcommandservice.cpp
SOURCES += services/killservice.cpp
SOURCES += services/listappsservice.cpp
SOURCES += services/confservice.cpp
SOURCES += services/foregroundservice.cpp
SOURCES += services/systeminfoservice.cpp
SOURCES += services/resourceloggingservice.cpp
SOURCES += services/shelltask.cpp
SOURCES += services/startedappservice.cpp

unix:!macx:!CONFIG(no_x) {
   SOURCES += services/uicommandservice_unix.cpp
} else {
    win32: {
        SOURCES += services/uicommandservice_windows.cpp
    } else {
        SOURCES += services/uicommandservice.cpp
    }
}
