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

include(tasbase.pri)

TEMPLATE = subdirs

SUBDIRS += tascore \
           qttasserver \
           qttasserver_ui \
           fixtures \
           traversers \
           loaders \
           utilityapp

fixtures.depends = tascore
traversers.depends = tascore

unix:!macx {
#  EXAMPLES = doc/
  EXAMPLES.files = doc/*
  EXAMPLES.path = /usr/share/doc/qttas-dev/
  DESKTOP.files = etc/qttas_ui.desktop
  DESKTOP.path = /usr/share/applications/
  INSTALLS += EXAMPLES DESKTOP
  # TODO other platforms to support?

  MKSPECS.files = mkspecs/features/qttas_traverser.prf mkspecs/features/qttas_fixture.prf
  MKSPECS.path = $$[QT_INSTALL_DATA]/mkspecs/features/
  INSTALLS += MKSPECS
}
