#-------------------------------------------------
#
# Project created by QtCreator 2017-08-15T17:41:18
#
#-------------------------------------------------

QT       += core gui svg concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = studio
TEMPLATE = app
DESTDIR = bin

GAMS_CORE_TMP = $$(GAMS_CORE_PATH)
!exists($$PWD/gamsinclude.pri) {
    equals(GAMS_CORE_TMP, "") {
        macx {
            GAMSINC = GAMS_DISTRIB=/Applications/GAMS24.9/sysdir \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        unix:!macx {
            GAMSINC = GAMS_DISTRIB=$$(HOME)/gams/gams24.9_linux_x64_64_sfx \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        win32 {
            GAMSINC = GAMS_DISTRIB=C:/GAMS/win64/24.9 \
                      GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        }
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    } else {
        GAMSINC = GAMS_DISTRIB=$$(GAMS_CORE_PATH) \
                  GAMS_DISTRIB_API=\$$GAMS_DISTRIB/apifiles/C/api
        write_file($$PWD/gamsinclude.pri,GAMSINC)
    }
}
exists($$PWD/gamsinclude.pri) {
    include($$PWD/gamsinclude.pri)
}

unix:LIBS += -ldl
win32:LIBS += -luser32


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
    $$GAMS_DISTRIB_API/gclgms.c \
    $$GAMS_DISTRIB_API/gdxcc.c \
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
    fileactioncontext.cpp \
    newdialog.cpp \
    modeldialog/modeldialog.cpp \
    modeldialog/glbparser.cpp   \
    modeldialog/libraryitem.cpp \
    modeldialog/library.cpp     \
    modeldialog/librarymodel.cpp \
    gamsprocess.cpp     \
    gamslibprocess.cpp  \
    abstractprocess.cpp \
    filetype.cpp        \
    filemetrics.cpp \
    gdxviewer/gdxviewer.cpp \
    gdxviewer/gdxsymbol.cpp \
    gdxviewer/gdxsymboltable.cpp \
    gamspaths.cpp \
    filetreemodel.cpp \
    textmark.cpp \
    logger.cpp

HEADERS += \
    codeeditor.h \
    filesystemcontext.h \
    filecontext.h \
    filerepository.h \
    filegroupcontext.h \
    welcomepage.h \
    fileactioncontext.h \
    mainwindow.h \
    exception.h \
    treeitemdelegate.h \
    version.h \
    newdialog.h \
    modeldialog/modeldialog.h   \
    modeldialog/glbparser.h     \
    modeldialog/libraryitem.h   \
    modeldialog/library.h       \
    modeldialog/librarymodel.h \
    gamsprocess.h       \
    gamslibprocess.h    \
    abstractprocess.h   \
    filetype.h \
    filemetrics.h \
    gdxviewer/gdxviewer.h \
    gdxviewer/gdxsymbol.h \
    gdxviewer/gdxsymboltable.h \
    gamspaths.h \
    filetreemodel.h \
    textmark.h \
    logger.h

FORMS += \
    welcomepage.ui  \
    mainwindow.ui   \
    newdialog.ui    \
    modeldialog/modeldialog.ui \
    gdxviewer/gdxviewer.ui

INCLUDEPATH += \
    $$GAMS_DISTRIB_API

RESOURCES += \
    ../icons/icons.qrc

DISTFILES += \
    studio.rc
