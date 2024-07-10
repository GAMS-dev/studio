TEMPLATE = app

include(../tests.pri)

INCLUDEPATH +=  \
        $$SRCPATH \
        $$SRCPATH/help

HEADERS += \
    testdoclocation.h \
    $$SRCPATH/help/helpdata.h

SOURCES += \
    testdoclocation.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/exception.cpp
