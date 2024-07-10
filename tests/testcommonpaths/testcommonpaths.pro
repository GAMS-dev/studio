TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

HEADERS += \
    testcommonpaths.h

SOURCES += \
    testcommonpaths.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/exception.cpp
