#
# This file is part of the GAMS Studio project.
#
# Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
# Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

QT       += core gui svg concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webenginewidgets

TARGET = studio
TEMPLATE = app
DESTDIR = bin

CONFIG += c++14

# Setup and include the GAMS distribution
include($$PWD/gamsdependency.pri)

include (../studioversion)
DEFINES += 'STUDIO_VERSION=\\"$$VERSION\\"'

macx {
# ! The icns-file is created from a folder named gams.iconset containing images in multiple sizes.
# ! On mac osX type the command: iconutil -c icns [base-folder]/gams.iconset to create gams.icns
    ICON = studio.icns
    QMAKE_INFO_PLIST=Info.plist
}
unix {
    LIBS += -ldl
}
win32 {
    RC_FILE += studio.rc
    LIBS += -luser32
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _CRT_SECURE_NO_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
    logger.cpp \
    logcontext.cpp \
    gdxviewer/columnfilter.cpp \
    gdxviewer/columnfilterframe.cpp \
    gdxviewer/filteruelmodel.cpp \
    settingsdialog.cpp \
    studiosettings.cpp \
    application.cpp \
    projectcontextmenu.cpp \
    gdxviewer/gdxsymbolview.cpp \
    gdxviewer/gdxsymbolheaderview.cpp \
    option/option.cpp \
    option/commandlinehistory.cpp \
    option/commandlineoption.cpp \
    option/commandlinetokenizer.cpp \
    option/optionparametermodel.cpp \
    option/optioncompleterdelegate.cpp \
    option/lineeditcompleteevent.cpp \
    option/optiondefinitionitem.cpp \
    option/optiondefinitionmodel.cpp \
    option/optioneditor.cpp \
    option/optionsortfilterproxymodel.cpp \
    syntax/textmark.cpp \
    syntax/textmarklist.cpp \
    syntax/syntaxhighlighter.cpp \
    syntax/syntaxformats.cpp \
    syntax/syntaxdeclaration.cpp \
    syntax/syntaxidentifier.cpp \
    searchwidget.cpp \
    resultsview.cpp \
    searchresultlist.cpp \
    keys.cpp \
    helpview.cpp \
    bookmarkdialog.cpp \
    commandlineparser.cpp \
    wplabel.cpp \
    gotowidget.cpp \
    logeditor.cpp \
    abstracteditor.cpp

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
    logger.h \
    logcontext.h \
    gdxviewer/columnfilter.h \
    gdxviewer/columnfilterframe.h \
    gdxviewer/filteruelmodel.h \
    settingsdialog.h \
    studiosettings.h \
    application.h \
    projectcontextmenu.h \
    gdxviewer/gdxsymbolview.h \
    gdxviewer/gdxsymbolheaderview.h \
    option/option.h \
    option/commandlinehistory.h \
    option/commandlinetokenizer.h \
    option/commandlineoption.h \
    option/optionparametermodel.h \
    option/optioncompleterdelegate.h \
    option/lineeditcompleteevent.h \
    option/optiondefinitionitem.h \
    option/optiondefinitionmodel.h \
    option/optioneditor.h \
    option/optionsortfilterproxymodel.h \
    syntax.h \
    syntax/textmark.h \
    syntax/textmarklist.h \
    syntax/syntaxhighlighter.h \
    syntax/syntaxformats.h \
    syntax/syntaxdeclaration.h \
    syntax/syntaxidentifier.h \
    syntax/syntaxdata.h \
    searchwidget.h \
    resultsview.h \
    searchresultlist.h \
    syntax/syntaxdata.h \
    keys.h \
    helpview.h \
    bookmarkdialog.h \
    commandlineparser.h \
    wplabel.h \
    gotowidget.h \
    logeditor.h \
    abstracteditor.h

FORMS += \
    welcomepage.ui  \
    mainwindow.ui   \
    newdialog.ui    \
    modeldialog/modeldialog.ui \
    gdxviewer/gdxviewer.ui \
    gdxviewer/columnfilterframe.ui \
    gdxviewer/gdxsymbolview.ui \
    settingsdialog.ui \
    option/optionconfigurator.ui \
    searchwidget.ui \
    resultsview.ui \
    bookmarkdialog.ui \
    gotowidget.ui

RESOURCES += \
    ../icons/icons.qrc

DISTFILES += \
    studio.rc
