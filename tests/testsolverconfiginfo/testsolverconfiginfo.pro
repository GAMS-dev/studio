TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH            \
               $$SRCPATH/editors    \
               $$SRCPATH/support

HEADERS +=                  \
    testsolverconfiginfo.h

SOURCES +=                                       \
    testsolverconfiginfo.cpp                     \
    $$SRCPATH/editors/defaultsystemlogger.cpp    \
    $$SRCPATH/editors/sysloglocator.cpp          \
    $$SRCPATH/support/solverconfiginfo.cpp       \
    $$SRCPATH/commonpaths.cpp                    \
    $$SRCPATH/exception.cpp
