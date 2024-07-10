TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH                        \
               $$SRCPATH/editors                \
               $$SRCPATH/syntax

HEADERS +=                                      \
    syntaxsimulator.h                           \
    testcompleter.h                             \
    $$SRCPATH/editors/codecompleter.h           \
    $$SRCPATH/editors/sysloglocator.h           \
    $$SRCPATH/editors/defaultsystemlogger.h     \
    $$SRCPATH/syntax/syntaxformats.h            \
    $$SRCPATH/common.h                          \
    $$SRCPATH/theme.h                           \
    $$SRCPATH/svgengine.h                       \
    $$SRCPATH/exception.h                       \
    $$SRCPATH/logger.h

SOURCES +=                                      \
    syntaxsimulator.cpp                         \
    testcompleter.cpp                           \
    $$SRCPATH/editors/codecompleter.cpp         \
    $$SRCPATH/editors/sysloglocator.cpp         \
    $$SRCPATH/editors/defaultsystemlogger.cpp   \
    $$SRCPATH/syntax/syntaxformats.cpp          \
    $$SRCPATH/theme.cpp                         \
    $$SRCPATH/svgengine.cpp                     \
    $$SRCPATH/exception.cpp                     \
    $$SRCPATH/logger.cpp
