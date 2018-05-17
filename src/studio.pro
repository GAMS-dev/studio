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

include (../version)
DEFINES += 'STUDIO_VERSION=\\"$$VERSION\\"'
DEFINES += 'STUDIO_MAJOR_VERSION=$$STUDIO_MAJOR_VERSION'
DEFINES += 'STUDIO_MINOR_VERSION=$$STUDIO_MINOR_VERSION'
DEFINES += 'STUDIO_PATCH_LEVEL=$$STUDIO_PATCH_LEVEL'
DEFINES += 'GAMS_DISTRIB_VERSION=$$GAMS_DISTRIB_VERSION'
DEFINES += 'GAMS_DISTRIB_VERSION_SHORT=\\"'$$GAMS_DISTRIB_MAJOR'.'$$GAMS_DISTRIB_MINOR'\\"'
DEFINES += 'GAMS_DISTRIB_VERSION_NEXT_SHORT=\\"'$$GAMS_DISTRIB_NEXT_MAJOR'.'$$GAMS_DISTRIB_NEXT_MINOR'\\"'

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
    abstractprocess.cpp \
    application.cpp \
    autosavehandler.cpp \
    bookmarkdialog.cpp \
    checkforupdatewrapper.cpp \
    commandlineparser.cpp \
    commonpaths.cpp \
    distributionvalidator.cpp \
    editors/abstracteditor.cpp \
    editors/codeeditor.cpp \
    editors/logeditor.cpp \
    editors/selectencodings.cpp \
    exception.cpp \
    file/filecontext.cpp \
    file/filegroupcontext.cpp \
    file/filemetrics.cpp \
    file/filerepository.cpp \
    file/filesystemcontext.cpp \
    file/filetreemodel.cpp \
    file/filetype.cpp \
    file/logcontext.cpp \
    gamslibprocess.cpp  \
    gamsprocess.cpp     \
    gdxviewer/columnfilter.cpp \
    gdxviewer/columnfilterframe.cpp \
    gdxviewer/filteruelmodel.cpp \
    gdxviewer/gdxsymbol.cpp \
    gdxviewer/gdxsymbolheaderview.cpp \
    gdxviewer/gdxsymboltable.cpp \
    gdxviewer/gdxsymbolview.cpp \
    gdxviewer/gdxviewer.cpp \
    gotowidget.cpp \
    helpview.cpp \
    keys.cpp \
    logger.cpp \
    lxiviewer/lxiparser.cpp \
    lxiviewer/lxitreeitem.cpp \
    lxiviewer/lxitreemodel.cpp \
    lxiviewer/lxiviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    modeldialog/glbparser.cpp   \
    modeldialog/library.cpp     \
    modeldialog/libraryitem.cpp \
    modeldialog/librarymodel.cpp \
    modeldialog/modeldialog.cpp \
    option/addoptionheaderview.cpp \
    option/commandlinehistory.cpp \
    option/commandlineoption.cpp \
    option/commandlinetokenizer.cpp \
    option/lineeditcompleteevent.cpp \
    option/option.cpp \
    option/optioncompleterdelegate.cpp \
    option/optiondefinitionitem.cpp \
    option/optiondefinitionmodel.cpp \
    option/optioneditor.cpp \
    option/optionparametermodel.cpp \
    option/optionsortfilterproxymodel.cpp \
    projectcontextmenu.cpp \
    resultsview.cpp \
    searchresultlist.cpp \
    searchwidget.cpp \
    settingsdialog.cpp \
    statuswidgets.cpp \
    studiosettings.cpp \
    syntax/syntaxdeclaration.cpp \
    syntax/syntaxformats.cpp \
    syntax/syntaxhighlighter.cpp \
    syntax/syntaxidentifier.cpp \
    syntax/textmark.cpp \
    syntax/textmarklist.cpp \
    treeitemdelegate.cpp \
    updatedialog.cpp \
    welcomepage.cpp \
    wplabel.cpp

HEADERS += \
    abstractprocess.h \
    application.h \
    autosavehandler.h \
    bookmarkdialog.h \
    checkforupdatewrapper.h \
    commandlineparser.h \
    commonpaths.h \
    distributionvalidator.h \
    editors/abstracteditor.h \
    editors/codeeditor.h \
    editors/logeditor.h \
    editors/selectencodings.h \
    exception.h \
    file.h \
    file/filecontext.h \
    file/filegroupcontext.h \
    file/filemetrics.h \
    file/filerepository.h \
    file/filesystemcontext.h \
    file/filetreemodel.h \
    file/filetype.h \
    file/logcontext.h \
    gamslibprocess.h \
    gamsprocess.h \
    gdxviewer/columnfilter.h \
    gdxviewer/columnfilterframe.h \
    gdxviewer/filteruelmodel.h \
    gdxviewer/gdxsymbol.h \
    gdxviewer/gdxsymbolheaderview.h \
    gdxviewer/gdxsymboltable.h \
    gdxviewer/gdxsymbolview.h \
    gdxviewer/gdxviewer.h \
    gotowidget.h \
    helpview.h \
    keys.h \
    logger.h \
    lxiviewer/lxiparser.h \
    lxiviewer/lxitreeitem.h \
    lxiviewer/lxitreemodel.h \
    lxiviewer/lxiviewer.h \
    mainwindow.h \
    modeldialog/glbparser.h \
    modeldialog/library.h \
    modeldialog/libraryitem.h \
    modeldialog/librarymodel.h \
    modeldialog/modeldialog.h \
    option/addoptionheaderview.h \
    option/commandlinehistory.h \
    option/commandlineoption.h \
    option/commandlinetokenizer.h \
    option/lineeditcompleteevent.h \
    option/option.h \
    option/optioncompleterdelegate.h \
    option/optiondefinitionitem.h \
    option/optiondefinitionmodel.h \
    option/optioneditor.h \
    option/optionparametermodel.h \
    option/optionsortfilterproxymodel.h \
    projectcontextmenu.h \
    resultsview.h \
    searchresultlist.h \
    searchwidget.h \
    settingsdialog.h \
    statuswidgets.h \
    studiosettings.h \
    syntax.h \
    syntax/syntaxdata.h \
    syntax/syntaxdata.h \
    syntax/syntaxdeclaration.h \
    syntax/syntaxformats.h \
    syntax/syntaxhighlighter.h \
    syntax/syntaxidentifier.h \
    syntax/textmark.h \
    syntax/textmarklist.h \
    treeitemdelegate.h \
    updatedialog.h \
    version.h \
    welcomepage.h \
    wplabel.h

FORMS += \
    bookmarkdialog.ui \
    editors/selectencodings.ui \
    gdxviewer/columnfilterframe.ui \
    gdxviewer/gdxsymbolview.ui \
    gdxviewer/gdxviewer.ui \
    gotowidget.ui \
    lxiviewer/lxiviewer.ui \
    mainwindow.ui \
    modeldialog/modeldialog.ui \
    newdialog.ui \
    resultsview.ui \
    searchwidget.ui \
    settingsdialog.ui \
    updatedialog.ui \
    welcomepage.ui

RESOURCES += \
    ../icons/icons.qrc

DISTFILES += \
    studio.rc
