TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH            \
               $$SRCPATH/editors    \
               $$SRCPATH/process

HEADERS +=                                      \
    testabstractprocess.h                       \
    $$SRCPATH/editors/abstractsystemlogger.h    \
    $$SRCPATH/editors/defaultsystemlogger.h     \
    $$SRCPATH/editors/SysLogLocator.h           \
    $$SRCPATH/process/abstractprocess.h         \
    $$SRCPATH/commonpaths.h                     \
    $$SRCPATH/common.h                          \
    $$SRCPATH/exception.h

SOURCES +=                                      \
    tst_testgamsgetkeyprocess.cpp               \
    $$SRCPATH/editors/defaultsystemlogger.cpp   \
    $$SRCPATH/editors/SysLogLocator.cpp         \
    $$SRCPATH/process/gamsgetkeyprocess.cpp      \
    $$SRCPATH/process/abstractprocess.cpp       \
    $$SRCPATH/commonpaths.cpp                   \
    $$SRCPATH/common.cpp                        \
    $$SRCPATH/exception.cpp

