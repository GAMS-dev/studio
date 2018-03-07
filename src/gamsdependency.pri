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

# GAMS_CORE_PATH is Jenkins build switch
GAMS_CORE_TMP = $$(GAMS_CORE_PATH)
!exists($$PWD/gamsinclude.pri) {
    equals(GAMS_CORE_TMP, "") {
        macx {
            GAMSINC = GAMS_DISTRIB=/Applications/GAMS25.1/sysdir \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        unix:!macx {
            GAMSINC = GAMS_DISTRIB=$$(HOME)/gams/gams25.1_linux_x64_64_sfx \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        win32 {
            GAMSINC = GAMS_DISTRIB=C:/GAMS/win64/25.1 \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    } else {
        GAMSINC = GAMS_DISTRIB=$$(GAMS_CORE_PATH) \
                  GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    }
}
exists($$PWD/gamsinclude.pri) {
    include($$PWD/gamsinclude.pri)
}

INCLUDEPATH += $$GAMS_DISTRIB_API

SOURCES += \
    $$GAMS_DISTRIB_API/c4umcc.c \
    $$GAMS_DISTRIB_API/gclgms.c \
    $$GAMS_DISTRIB_API/gdxcc.c  \
    $$GAMS_DISTRIB_API/optcc.c
