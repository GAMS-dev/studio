#-------------------------------------------------
#
# Project created by QtCreator 2017-08-15T17:41:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ide
TEMPLATE = app
DESTDIR = bin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    codeeditor.cpp \
    tabwidget.cpp \
    filesystemcontext.cpp \
    filecontext.cpp \
    filerepository.cpp \
    filegroupcontext.cpp \
    welcomepage.cpp \
    editor.cpp \
    modeldialog.cpp \
    mainwindow.cpp

HEADERS += \
    codeeditor.h \
    tabwidget.h \
    filesystemcontext.h \
    filecontext.h \
    filerepository.h \
    filegroupcontext.h \
    welcomepage.h \
    editor.h \
    modeldialog.h \
    mainwindow.h

FORMS += \
    welcomepage.ui \
    editor.ui \
    modeldialog.ui \
    mainwindow.ui


RESOURCES += \
    icons/icons.qrc
