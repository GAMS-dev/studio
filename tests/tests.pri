#
# This file is part of the GAMS Studio project.
#
# Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
# Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

QT += core testlib
QT -= gui

CONFIG += c++14
CONFIG -= app_bundle

DESTDIR = ../bin

include (../version)
DEFINES += 'STUDIO_VERSION=\\"$$VERSION\\"'
DEFINES += 'STUDIO_MAJOR_VERSION=$$STUDIO_MAJOR_VERSION'
DEFINES += 'STUDIO_MINOR_VERSION=$$STUDIO_MINOR_VERSION'
DEFINES += 'STUDIO_PATCH_LEVEL=$$STUDIO_PATCH_LEVEL'
DEFINES += 'GAMS_DISTRIB_VERSION=$$GAMS_DISTRIB_VERSION'
DEFINES += 'GAMS_DISTRIB_VERSION_SHORT=\\"'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR'\\"'
DEFINES += 'GAMS_DISTRIB_VERSION_NEXT_SHORT=\\"'$$GAMS_DISTRIB_NEXT_MAJOR'.'$$GAMS_DISTRIB_NEXT_MINOR'\\"'

TESTSROOT = $$_PRO_FILE_PWD_/..
