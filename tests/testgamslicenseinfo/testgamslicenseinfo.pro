TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH            \
               $$SRCPATH/editors    \
               $$SRCPATH/support

HEADERS +=                  \
    testgamslicenseinfo.h

SOURCES +=                                      \
    testgamslicenseinfo.cpp                     \
    $$SRCPATH/editors/defaultsystemlogger.cpp   \
    $$SRCPATH/editors/sysloglocator.cpp         \
    $$SRCPATH/support/solverconfiginfo.cpp      \
    $$SRCPATH/support/gamslicenseinfo.cpp       \
    $$SRCPATH/commonpaths.cpp                   \
    $$SRCPATH/exception.cpp
