TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH            \
               $$SRCPATH/editors    \
               $$SRCPATH/support    \
               $$PWD/../extern
include(../../extern/yaml-cpp/yaml-cpp.pri)

HEADERS +=                                      \
    $$SRCPATH/editors/defaultsystemlogger.h     \
    $$SRCPATH/editors/sysloglocator.h           \
    $$SRCPATH/exception.h                       \
    $$SRCPATH/commonpaths.h                     \
    $$SRCPATH/commandlineparser.h               \
    $$SRCPATH/support/versioninfoloader.h

SOURCES +=                                      \
    $$SRCPATH/editors/defaultsystemlogger.cpp   \
    $$SRCPATH/editors/sysloglocator.cpp         \
    $$SRCPATH/support/versioninfoloader.cpp     \
    $$SRCPATH/exception.cpp                     \
    $$SRCPATH/commonpaths.cpp                   \
    $$SRCPATH/commandlineparser.cpp             \
    tst_testversioninfoloader.cpp
