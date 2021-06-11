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

TEMPLATE = app

include(../tests.pri)

INCLUDEPATH +=  \
        $$SRCPATH \
        $$SRCPATH/option

HEADERS += \
    testgamsoption.h \
    $$SRCPATH/theme.h \
    $$SRCPATH/SvgEngine.h \
    $$SRCPATH/option/optiontokenizer.h \
    $$SRCPATH/option/option.h

SOURCES += \
    testgamsoption.cpp \
    $$SRCPATH/theme.cpp \
    $$SRCPATH/SvgEngine.cpp \
    $$SRCPATH/option/optiontokenizer.cpp \
    $$SRCPATH/option/option.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/support/solverconfiginfo.cpp \
    $$SRCPATH/exception.cpp
