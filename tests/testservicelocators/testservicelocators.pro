TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

HEADERS += \
    testsysloglocator.h

SOURCES += \
    testsysloglocator.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp
