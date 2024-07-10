TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH \
               $$SRCPATH/syntax

HEADERS += \
    testblockcode.h \
    $$SRCPATH/syntax/blockcode.h \
    $$SRCPATH/syntax/syntaxformats.h

SOURCES += \
    testblockcode.cpp \
    $$SRCPATH/logger.cpp
