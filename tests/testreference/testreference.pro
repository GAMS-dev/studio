TEMPLATE = app

include(../tests.pri)

INCLUDEPATH +=  \
        $$SRCPATH

HEADERS += \
    testreference.h

SOURCES += \
    testreference.cpp
