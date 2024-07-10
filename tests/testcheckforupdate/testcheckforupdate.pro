TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH            \
               $$SRCPATH/editors    \
               $$SRCPATH/support    \
               $$PWD/../extern
include(../../extern/yaml-cpp/yaml-cpp.pri)

HEADERS +=                          \
    $$SRCPATH/editors/defaultsystemlogger.h   \
    $$SRCPATH/editors/sysloglocator.h         \
    $$SRCPATH/support/versioninfoloader.h     \
    $$SRCPATH/support/gamslicenseinfo.h       \
    $$SRCPATH/support/checkforupdate.h        \
    $$SRCPATH/support/solverconfiginfo.h      \
    $$SRCPATH/commonpaths.h                   \
    $$SRCPATH/exception.h                     \
    testcheckforupdate.h

SOURCES +=                                      \
    $$SRCPATH/editors/defaultsystemlogger.cpp   \
    $$SRCPATH/editors/sysloglocator.cpp         \
    $$SRCPATH/support/versioninfoloader.cpp     \
    $$SRCPATH/support/gamslicenseinfo.cpp       \
    $$SRCPATH/support/checkforupdate.cpp        \
    $$SRCPATH/support/solverconfiginfo.cpp      \
    $$SRCPATH/commonpaths.cpp                   \
    $$SRCPATH/exception.cpp                     \
    testcheckforupdate.cpp
