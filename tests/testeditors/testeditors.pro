TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

HEADERS += \
    testcodeedit.h \
    $$SRCPATH/editors/editorhelper.h

SOURCES += \
    testcodeedit.cpp \
    $$SRCPATH/editors/editorhelper.cpp
