#
# This file is part of the GAMS Studio project.
#
# Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
# Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

QT += core testlib gui widgets svg printsupport

CONFIG += c++14
CONFIG -= app_bundle

DESTDIR = ../bin

# Setup and include the GAMS distribution
include(../gamsdependency.pri)

macx {
    HEADERS += ../../platform/macos/macospathfinder.h   \
               ../../platform/macos/macoscocoabridge.h

    SOURCES += ../../platform/macos/macospathfinder.cpp

    OBJECTIVE_SOURCES += ../../platform/macos/macoscocoabridge.mm

    LIBS += -framework AppKit
}
unix {
    LIBS += -ldl
}
win32 {
    LIBS += -luser32
}

TESTSROOT = $$_PRO_FILE_PWD_/..
SRCPATH = $$TESTSROOT/../src
