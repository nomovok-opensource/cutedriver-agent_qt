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


# This file includes qjson parameters for qttasserver.
#
# See qjson/COPYING for license information
# Original source avaiable from:
# http://gitorious.org/qjson/qjson/trees/0.7.1

message(qjson pri)

unix:!symbian:!mac {
  # just use default library of the distribution, assume it's been installed
    message(qjson unix)
    LIBS += -lqjson
}

win32: {
    message(qjson win32)
    LIBS += -L$$PWD/qjson/lib -lqjson
    INCLUDEPATH += $$PWD
    message($$PWD LIBS $$LIBS)
    message($$PWD INCLUDEPATH $$INCLUDEPATH)
}

