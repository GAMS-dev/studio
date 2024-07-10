TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += \
            $$SRCPATH \
            $$SRCPATH/connect

HEADERS += \
    testconnect.h \
    $$SRCPATH/connect/connectdataitem.h

SOURCES += \
    testconnect.cpp \
    $$SRCPATH/connect/connectdataitem.cpp
