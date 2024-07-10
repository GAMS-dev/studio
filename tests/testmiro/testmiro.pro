TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

HEADERS += \
    testmirocommon.h

SOURCES += \
    testmirocommon.cpp \
    $$SRCPATH/miro/mirocommon.cpp
