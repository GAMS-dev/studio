#
# This file is part of the GAMS Studio project.
#
# Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
# Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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

# Set this to "false" to build Studio without QWebEngine enabled,
# which deactivates the studio help view.
QWEBENGINE=true

unix {
    QMAKE_CFLAGS += -Wimplicit-fallthrough=0
}
win32 {
    # Switch off warings caused by GAMS headers
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

include (version)
# GAMS_CORE_PATH is Jenkins build switch
GAMS_CORE_TMP = $$(GAMS_CORE_PATH)
!exists($$PWD/gamsinclude.pri) {
    equals(GAMS_CORE_TMP, "") {
        macx {
            GAMSINC = GAMS_DISTRIB=/Library/Frameworks/GAMS.framework/Versions/'$$GAMS_DISTRIB_MAJOR'/Resources \
                      GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api   \
                      GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        }
        unix:!macx {
            GAMSINC = GAMS_DISTRIB=$$(HOME)/gams/gams'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR'_linux_x64_64_sfx \
                      GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api   \
                      GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        }
        win32 {
            GAMSINC = GAMS_DISTRIB=C:/GAMS/'$$GAMS_DISTRIB_MAJOR' \
                      GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api   \
                      GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        }
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    } else {
        GAMSINC = GAMS_DISTRIB=$$(GAMS_CORE_PATH)   \
                  GAMS_DISTRIB_C_API=\$$GAMS_DISTRIB/apifiles/C/api       \
                  GAMS_DISTRIB_CPP_API=\$$GAMS_DISTRIB/apifiles/C++/api
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    }
}
exists($$PWD/gamsinclude.pri) {
    include($$PWD/gamsinclude.pri)
    macx {
        DEFINES += 'GAMS_DISTRIB_PATH=\\"$$GAMS_DISTRIB\\"'
    }
    else {
        DEFINES += 'GAMS_DISTRIB_PATH=\\"\\"'
    }
}

INCLUDEPATH += $$GAMS_DISTRIB_C_API     \
               $$GAMS_DISTRIB_CPP_API

SOURCES +=                          \
    $$GAMS_DISTRIB_C_API/c4umcc.c   \
    $$GAMS_DISTRIB_C_API/palmcc.c   \
    $$GAMS_DISTRIB_C_API/gdxcc.c    \
    $$GAMS_DISTRIB_C_API/optcc.c    \
    $$GAMS_DISTRIB_C_API/cfgmcc.c   \
    $$GAMS_DISTRIB_C_API/guccc.c    \
    $$GAMS_DISTRIB_C_API/gucapi.c
