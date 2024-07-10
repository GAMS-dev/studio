TEMPLATE = app

include(../tests.pri)

INCLUDEPATH +=  \
        $$SRCPATH \
        $$SRCPATH/option

HEADERS += \
    testgamsoption.h \
    $$SRCPATH/theme.h \
    $$SRCPATH/SvgEngine.h \
    $$SRCPATH/option/optiontokenizer.h \
    $$SRCPATH/option/option.h

SOURCES += \
    testgamsoption.cpp \
    $$SRCPATH/theme.cpp \
    $$SRCPATH/SvgEngine.cpp \
    $$SRCPATH/option/optiontokenizer.cpp \
    $$SRCPATH/option/option.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/support/solverconfiginfo.cpp \
    $$SRCPATH/exception.cpp
