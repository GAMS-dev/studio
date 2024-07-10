TEMPLATE = app

include(../tests.pri)

HEADERS +=                          \
    testabstractprocess.h           \
    $$SRCPATH/process/abstractprocess.h     \
    $$SRCPATH/commonpaths.h         \
    $$SRCPATH/exception.h

SOURCES +=                          \
    testabstractprocess.cpp         \
    $$SRCPATH/process/abstractprocess.cpp   \
    $$SRCPATH/commonpaths.cpp       \
    $$SRCPATH/exception.cpp
