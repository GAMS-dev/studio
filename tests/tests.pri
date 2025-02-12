QT += core testlib widgets gui svg concurrent network printsupport

CONFIG += c++14
CONFIG -= app_bundle

DESTDIR = ../bin

OBJECTS_DIR=../../objects
MOC_DIR=../../objects

# Setup and include the GAMS distribution
include(../gamsdependency.pri)

macx {
    HEADERS += ../../platform/macos/macospathfinder.h   \
               ../../platform/macos/macoscocoabridge.h

    SOURCES += ../../platform/macos/macospathfinder.cpp

    OBJECTIVE_SOURCES += ../../platform/macos/macoscocoabridge.mm

    LIBS += -framework AppKit
}
unix {
    LIBS += -ldl
}
win32 {
    LIBS += -luser32
}

TESTSROOT = $$_PRO_FILE_PWD_/..
SRCPATH = $$TESTSROOT/../src
