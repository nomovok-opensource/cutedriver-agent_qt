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

SOURCES += $$PWD/cucumberwireprotocolserver.cpp \
    $$PWD/cucumberapplicationmanager.cpp

HEADERS += $$PWD/cucumberwireprotocolserver.h\
    $$PWD/cucumberapplicationmanager.h

include(../../3rdparty/qjson.pri)
