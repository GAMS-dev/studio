TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH \
               $$SRCPATH/editors

HEADERS += \
    $$SRCPATH/editors/fastfilemapper.h \
    $$SRCPATH/editors/abstracttextmapper.h \
    $$SRCPATH/theme.h \
    $$SRCPATH/svgengine.h \
    testfilemapper.h

SOURCES += \
    $$SRCPATH/editors/fastfilemapper.cpp \
    $$SRCPATH/editors/abstracttextmapper.cpp \
    $$SRCPATH/theme.cpp \
    $$SRCPATH/svgengine.cpp \
    $$SRCPATH/exception.cpp \
    $$SRCPATH/logger.cpp \
    testfilemapper.cpp

