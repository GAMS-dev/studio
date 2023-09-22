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
    $$SRCPATH/support/versioninfoloader.h

SOURCES +=                                      \
    $$SRCPATH/editors/defaultsystemlogger.cpp   \
    $$SRCPATH/editors/sysloglocator.cpp         \
    $$SRCPATH/support/versioninfoloader.cpp     \
    tst_testversioninfoloader.cpp
