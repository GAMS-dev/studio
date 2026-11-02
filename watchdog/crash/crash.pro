QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG -= qml_debug
CONFIG += force_debug_info
win32-msvc: QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO += /OPT:ICF
unix:       CONFIG += separate_debug_info
win32 {
LIBS += -lDbghelp
}

HEADERS += \
    ../../src/crashhandler.h

SOURCES += \
    main.cpp \
    ../../src/crashhandler.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
