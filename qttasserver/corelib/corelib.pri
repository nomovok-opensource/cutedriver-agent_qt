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

HEADERS += corelib/tasclientmanager.h
HEADERS += corelib/tasserver.h
HEADERS += corelib/tasservercommand.h
HEADERS += corelib/tasnativeutils.h

HEADERS += corelib/tasserverservicemanager.h

SOURCES += corelib/tasclientmanager.cpp
SOURCES += corelib/tasserver.cpp
SOURCES += corelib/tasservercommand.cpp
SOURCES += corelib/tasserverservicemanager.cpp


unix:{
        symbian: {
            SOURCES += corelib/tasnativeutils_symbian.cpp
        }
        else{
                !macx:!CONFIG(no_x) {
                SOURCES += corelib/tasnativeutils_unix.cpp
                }
                else {
                SOURCES += corelib/tasnativeutils.cpp
        }
        }
}
win32: {
    SOURCES += corelib/tasnativeutils_win.cpp
    LIBS += -lPsapi
}

