TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH \
               $$SRCPATH/file \
               $$SRCPATH/option

HEADERS += \
    testoptionapi.h \
    $$SRCPATH/common.h \
    $$SRCPATH/file/filetype.h


SOURCES += \
    testoptionapi.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/file/filetype.cpp \
    $$SRCPATH/exception.cpp \
    $$SRCPATH/support/solverconfiginfo.cpp  \
    $$SRCPATH/commandlineparser.cpp
