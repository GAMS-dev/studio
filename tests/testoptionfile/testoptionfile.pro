TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH \
               $$SRCPATH/option

HEADERS += \
    testoptionfile.h \
    $$SRCPATH/logger.h \
    $$SRCPATH/theme.h \
    $$SRCPATH/svgengine.h \
    $$SRCPATH/encoding.h \
    $$SRCPATH/option/option.h \
    $$SRCPATH/option/optiontokenizer.h

SOURCES += \
    testoptionfile.cpp \
    $$SRCPATH/logger.cpp \
    $$SRCPATH/theme.cpp \
    $$SRCPATH/svgengine.cpp \
    $$SRCPATH/encoding.cpp \
    $$SRCPATH/option/option.cpp \
    $$SRCPATH/option/optiontokenizer.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/support/solverconfiginfo.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/exception.cpp

OTHER_FILES +=         \
    optdummy.def       \
    genoptdeffile.cpp
