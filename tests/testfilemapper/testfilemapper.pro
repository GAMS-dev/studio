TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH \
               $$SRCPATH/editors

HEADERS += \
    $$SRCPATH/editors/fastfilemapper.h \
    $$SRCPATH/editors/abstracttextmapper.h \
    $$SRCPATH/encoding.h \
    $$SRCPATH/theme.h \
    $$SRCPATH/svgengine.h \
    $$SRCPATH/commonpaths.h                   \
    $$SRCPATH/exception.h                     \
    $$SRCPATH/commandlineparser.h             \
    testfilemapper.h

SOURCES += \
    $$SRCPATH/editors/fastfilemapper.cpp \
    $$SRCPATH/editors/abstracttextmapper.cpp \
    $$SRCPATH/encoding.cpp \
    $$SRCPATH/theme.cpp \
    $$SRCPATH/svgengine.cpp \
    $$SRCPATH/logger.cpp \
    $$SRCPATH/commonpaths.cpp                   \
    $$SRCPATH/exception.cpp                     \
    $$SRCPATH/commandlineparser.cpp             \
    testfilemapper.cpp

