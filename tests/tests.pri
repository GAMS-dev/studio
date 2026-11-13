QT += core testlib widgets gui svg concurrent network printsupport

CONFIG += c++17 warn_on depend_includepath testcase
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
    LIBS += -luser32 -lMpr
}
# The Studio objects in OBJECTS_DIR are reused and were compiled with Studio's precompiled
# header (src/studio.pro). With MSVC they can only be linked together with its object.
win32-msvc*:!no_pch {
    LIBS += ../../objects/studio_pch.obj
}

TESTSROOT = $$_PRO_FILE_PWD_/..
SRCPATH = $$TESTSROOT/../src
