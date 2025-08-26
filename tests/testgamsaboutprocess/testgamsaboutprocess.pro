TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH            \
               $$SRCPATH/editors    \
               $$SRCPATH/process

HEADERS +=                                      \
    $$SRCPATH/process/gamsaboutprocess.h        \
    $$SRCPATH/commonpaths.h                     \
    $$SRCPATH/common.h                          \
    $$SRCPATH/exception.h

SOURCES +=                                      \
    tst_testgamsaboutprocess.cpp                \
    $$SRCPATH/editors/defaultsystemlogger.cpp   \
    $$SRCPATH/editors/sysloglocator.cpp         \
    $$SRCPATH/process/gamsaboutprocess.cpp      \
    $$SRCPATH/commandlineparser.cpp             \
    $$SRCPATH/commonpaths.cpp                   \
    $$SRCPATH/common.cpp                        \
    $$SRCPATH/exception.cpp
