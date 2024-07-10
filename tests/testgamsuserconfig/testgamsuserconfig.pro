TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

HEADERS +=                  \
    testgamsuserconfig.h    \
    $$SRCPATH/option/gamsuserconfig.h   \
    $$SRCPATH/exception.h   \
    $$SRCPATH/file/filetype.h \
    $$SRCPATH/commonpaths.h

SOURCES +=                    \
    testgamsuserconfig.cpp                    \
    $$SRCPATH/option/gamsuserconfig.cpp       \
    $$SRCPATH/editors/sysloglocator.cpp       \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/file/filetype.cpp \
    $$SRCPATH/exception.cpp   \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/support/SolverConfigInfo.cpp \

