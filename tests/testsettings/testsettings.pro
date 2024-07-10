TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

HEADERS += \
    testsettings.h \
    $$SRCPATH/file/dynamicfile.h \
    $$SRCPATH/theme.h \
    $$SRCPATH/svgengine.h \
    $$SRCPATH/colors/palettemanager.h \
    $$SRCPATH/editors/sysloglocator.h \
    $$SRCPATH/editors/abstractsystemlogger.h
    $$SRCPATH/editors/defaultsystemlogger.h

SOURCES += \
    testsettings.cpp \
    $$SRCPATH/settings.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/logger.cpp \
    $$SRCPATH/exception.cpp \
    $$SRCPATH/file/dynamicfile.cpp \
    $$SRCPATH/theme.cpp \
    $$SRCPATH/svgengine.cpp \
    $$SRCPATH/colors/palettemanager.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp
