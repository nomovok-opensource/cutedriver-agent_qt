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

HEADERS += $$PWD/mousehandler.h
HEADERS += $$PWD/keyhandler.h
HEADERS += $$PWD/gesturehandler.h
#HEADERS += $$PWD/actionhandler.h
HEADERS += $$PWD/multitouchhandler.h
HEADERS += $$PWD/viewitemhandler.h

SOURCES += $$PWD/mousehandler.cpp 
SOURCES += $$PWD/keyhandler.cpp 
SOURCES += $$PWD/gesturehandler.cpp 
#SOURCES += $$PWD/actionhandler.cpp 
SOURCES += $$PWD/multitouchhandler.cpp 
SOURCES += $$PWD/viewitemhandler.cpp 

include ($$PWD/gestures/gestures.pri)
include ($$PWD/eventgenerator/eventgenerator.pri)
