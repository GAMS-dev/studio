#
# This file is part of the GAMS Studio project.
#
# Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
# Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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

QWEBENGINE=true
equals(QWEBENGINE, "true") {
DEFINES += QWEBENGINE
greaterThan(QT_MAJOR_VERSION, 4): QT += webenginewidgets
}

INCLUDEPATH += $$SRCPATH                \
               $$SRCPATH/modeldialog

HEADERS += $$files(*.h, true)

SOURCES += $$files(*.cpp, true)

#FORMS +=
