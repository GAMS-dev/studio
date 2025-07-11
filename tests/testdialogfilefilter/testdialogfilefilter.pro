TEMPLATE = app

include(../tests.pri)

INCLUDEPATH +=  \
        $$SRCPATH \
        $$SRCPATH/../extern \
        $$SRCPATH/editors

HEADERS += \
    testdialogfilefilter.h \
    $$SRCPATH/common.h     \
    $$SRCPATH/commonpaths.h \
    $$SRCPATH/viewhelper.h \
    $$SRCPATH/file/filetype.h

SOURCES += \
    testdialogfilefilter.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/common.cpp                        \
    $$SRCPATH/exception.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/file/filetype.cpp \
    $$SRCPATH/support/solverconfiginfo.cpp
