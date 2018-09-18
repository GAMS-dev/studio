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

include (version)
# GAMS_CORE_PATH is Jenkins build switch
GAMS_CORE_TMP = $$(GAMS_CORE_PATH)
!exists($$PWD/gamsinclude.pri) {
    equals(GAMS_CORE_TMP, "") {
        macx {
            GAMSINC = GAMS_DISTRIB=/Applications/GAMS'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR'/sysdir \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        unix:!macx {
            GAMSINC = GAMS_DISTRIB=$$(HOME)/gams/gams'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR'_linux_x64_64_sfx \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        win32 {
            GAMSINC = GAMS_DISTRIB=C:/GAMS/win64/'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR' \
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

# GAMS_BUILD is GAMS distrib build switch
GAMS_BUILD_ENV = $$(GAMS_BUILD)
equals(GAMS_BUILD_ENV, "") {
    INCLUDEPATH += $$GAMS_DISTRIB_API

    SOURCES += \
        $$GAMS_DISTRIB_API/c4umcc.c \
        $$GAMS_DISTRIB_API/gclgms.c \
        $$GAMS_DISTRIB_API/gdxcc.c  \
        $$GAMS_DISTRIB_API/optcc.c
} else {
    GSYS_ENV = $$(GSYS)
    equals(GSYS_ENV, "wei") {
        DEFINES += WEI
        DEFINES += CIA_WEX
    }
    equals(GSYS_ENV, "vs8") {
        DEFINES += VS8
        DEFINES += CIA_WIN
    }
    equals(GSYS_ENV, "leg") {
        DEFINES += LEG
        DEFINES += CIA_LEX
    }
    equals(GSYS_ENV, "deg") {
        DEFINES += DEG
        DEFINES += CIA_DEX
    }
    INCLUDEPATH += $$(GPRODUCTS)/gclib $$(GPRODUCTS)/apiwrap/gdxio $$(GPRODUCTS)/apiwrap/joat $$(GPRODUCTS)/apiwrap/optobj

    SOURCES = \
        $$(GPRODUCTS)/apiwrap/joat/c4umcc.c \
        $$(GPRODUCTS)/gclib/gclgms.c \
        $$(GPRODUCTS)/apiwrap/gdxio/gdxcc.c  \
        $$(GPRODUCTS)/apiwrap/optobj/optcc.c
}

