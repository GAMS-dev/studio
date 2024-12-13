TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH \
               $$SRCPATH/editors

HEADERS += \
    $$SRCPATH/editors/abstracttextmapper.h \
    $$SRCPATH/editors/chunktextmapper.h \
    $$SRCPATH/editors/logparser.h \
    $$SRCPATH/editors/memorymapper.h \
    $$SRCPATH/file/dynamicfile.h \
    $$SRCPATH/svgengine.h \
    $$SRCPATH/theme.h \
    $$SRCPATH/encoding.h \
    testmemorymapper.h

SOURCES += \
    $$SRCPATH/editors/abstracttextmapper.cpp \
    $$SRCPATH/editors/chunktextmapper.cpp \
    $$SRCPATH/editors/logparser.cpp \
    $$SRCPATH/editors/memorymapper.cpp \
    $$SRCPATH/file/dynamicfile.cpp \
    $$SRCPATH/exception.cpp \
    $$SRCPATH/logger.cpp \
    $$SRCPATH/svgengine.cpp \
    $$SRCPATH/theme.cpp \
    $$SRCPATH/encoding.cpp \
    testmemorymapper.cpp
