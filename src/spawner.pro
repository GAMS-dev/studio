TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= gui
DESTDIR = ../spawner

SOURCES += spawner.cpp \
    processthread.cpp

HEADERS += \
    processthread.h

