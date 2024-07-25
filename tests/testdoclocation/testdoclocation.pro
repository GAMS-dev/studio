TEMPLATE = app

include(../tests.pri)

INCLUDEPATH +=  \
        $$SRCPATH \
        $$SRCPATH/help

HEADERS += \
    testdoclocation.h \
    $$SRCPATH/help/helpdata.h

SOURCES += \
    testdoclocation.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/support/solverconfiginfo.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/exception.cpp
