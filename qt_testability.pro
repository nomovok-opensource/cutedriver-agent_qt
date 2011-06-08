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


TEMPLATE = subdirs

#for mac use "qmake -spec macx-g++" to avoid xcode files

SUBDIRS += 3rdparty \
    cucumber_wireprotocol

SUBDIRS += tascore

SUBDIRS += qttasserver

SUBDIRS += qttasserver_ui

SUBDIRS += fixtures

SUBDIRS += traversers

SUBDIRS += loaders

SUBDIRS += utilityapp

CONFIG(maemo) {
SUBDIRS  += xsession
}

CONFIG  += ordered

symbian: {
        SUBDIRS  += memlogsrv

        !CONFIG(no_mobility)  {
                CONFIG += mobility
        }

        SUBDIRS  += symbian
}

CONFIG(maemo){
        !CONFIG(no_mobility)  {
                CONFIG += mobility
        }
}

CONFIG(iby_export){
include(s60/s60.pri)
}

unix:!symbian {
#  EXAMPLES = doc/
  EXAMPLES.files = doc/*
  EXAMPLES.path = /usr/share/doc/qttas-dev/
  INSTALLS += EXAMPLES
  # TODO other platforms to support?

  MKSPECS.files = mkspecs/features/qttas_traverser.prf mkspecs/features/qttas_fixture.prf
  MKSPECS.path = $$[QT_INSTALL_DATA]/mkspecs/features/
  INSTALLS += MKSPECS
}
