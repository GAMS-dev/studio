#-------------------------------------------------
#
# Project created by QtCreator 2017-08-15T17:41:18
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = studio
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

win32: RC_FILE += studio.rc
macx: ICON = studio.icns
# ! The icns-file is created from a folder named gams.iconset containing images in multiple sizes.
# ! On mac osX type the command: iconutil -c icns [base-folder]/gams.iconset to create gams.icns

SOURCES += \
    main.cpp \
    codeeditor.cpp \
    filesystemcontext.cpp \
    filecontext.cpp \
    filerepository.cpp \
    filegroupcontext.cpp \
    welcomepage.cpp \
    mainwindow.cpp \
    treeitemdelegate.cpp \
    exception.cpp \
    gamsinfo.cpp \
    fileactioncontext.cpp \
    newdialog.cpp \
    modeldialog/modeldialog.cpp \
    modeldialog/glbparser.cpp   \
    modeldialog/libraryitem.cpp \
    modeldialog/library.cpp     \
    modeldialog/librarymodel.cpp \
    gamsprocess.cpp


HEADERS += \
    codeeditor.h \
    filesystemcontext.h \
    filecontext.h \
    filerepository.h \
    filegroupcontext.h \
    welcomepage.h \
    gamsinfo.h \
    fileactioncontext.h \
    newdialog.h \
    mainwindow.h \
    exception.h \
    treeitemdelegate.h \
    version.h \
    modeldialog/modeldialog.h   \
    modeldialog/glbparser.h     \
    modeldialog/libraryitem.h   \
    modeldialog/library.h       \
    modeldialog/librarymodel.h \
    gamsprocess.h

FORMS += \
    welcomepage.ui  \
    mainwindow.ui   \
    newdialog.ui    \
    modeldialog/modeldialog.ui

RESOURCES += \
    ../icons/icons.qrc

DISTFILES += \
    studio.rc
