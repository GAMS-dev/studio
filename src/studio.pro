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
    checkforupdatewrapper.cpp \
    commandlineparser.cpp \
    commonpaths.cpp \
    distributionvalidator.cpp \
    exception.cpp \
    file/filemetrics.cpp \
    file/filetype.cpp \
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
    help/bookmarkdialog.cpp \
    help/helppage.cpp \
    help/helptoolbar.cpp \
    help/helpview.cpp \
    help/helpwidget.cpp \
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
    option/commandlineoption.cpp \
    option/commandlinetokenizer.cpp \
    option/lineeditcompleteevent.cpp \
    option/option.cpp \
    option/optioncompleterdelegate.cpp \
    option/optiondefinitionitem.cpp \
    option/optiondefinitionmodel.cpp \
    option/optionsortfilterproxymodel.cpp \
    option/optiontablemodel.cpp \
    option/optionwidget.cpp \
    projectcontextmenu.cpp \
    resultsview.cpp \
    searchresultlist.cpp \
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
    wplabel.cpp \
    file/projectfilenode.cpp \
    file/projectgroupnode.cpp \
    file/projectrepo.cpp \
    file/projectabstractnode.cpp \
    file/projecttreemodel.cpp \
    file/projectlognode.cpp \
    file/filemetarepo.cpp \
    file/filemeta.cpp \
    gotodialog.cpp \
    editors/abstractedit.cpp \
    editors/processlogedit.cpp \
    editors/systemlogedit.cpp \
    encodingsdialog.cpp \
    editors/codeedit.cpp \
    syntax/systemloghighlighter.cpp \
    searchdialog.cpp \
    gamsproperties.cpp

HEADERS += \
    abstractprocess.h \
    application.h \
    autosavehandler.h \
    checkforupdatewrapper.h \
    commandlineparser.h \
    commonpaths.h \
    distributionvalidator.h \
    exception.h \
    file.h \
    file/filemetrics.h \
    file/filetype.h \
    file/projectabstractnode.h \
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
    help/bookmarkdialog.h \
    help/helppage.h \
    help/helptoolbar.h \
    help/helpview.h \
    help/helpwidget.h \
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
    option/commandlineoption.h \
    option/commandlinetokenizer.h \
    option/lineeditcompleteevent.h \
    option/option.h \
    option/optioncompleterdelegate.h \
    option/optiondefinitionitem.h \
    option/optiondefinitionmodel.h \
    option/optionsortfilterproxymodel.h \
    option/optiontablemodel.h \
    option/optionwidget.h \
    projectcontextmenu.h \
    resultsview.h \
    searchresultlist.h \
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
    wplabel.h \
    file/projectgroupnode.h \
    file/projectfilenode.h \
    file/projectlognode.h \
    file/projecttreemodel.h \
    file/filemetarepo.h \
    file/filemeta.h \
    file/projectrepo.h \
    gotodialog.h \
    editors/abstractedit.h \
    editors/processlogedit.h \
    editors/systemlogedit.h \
    encodingsdialog.h \
    editors/codeedit.h \
    syntax/systemloghighlighter.h \
    searchdialog.h \
    gamsproperties.h

FORMS += \
    gdxviewer/columnfilterframe.ui \
    gdxviewer/gdxsymbolview.ui \
    gdxviewer/gdxviewer.ui \
    help/bookmarkdialog.ui \
    help/helpwidget.ui \
    lxiviewer/lxiviewer.ui \
    mainwindow.ui \
    modeldialog/modeldialog.ui \
    option/optionwidget.ui \
    resultsview.ui \
    settingsdialog.ui \
    updatedialog.ui \
    welcomepage.ui \
    gotodialog.ui \
    encodingsdialog.ui \
    searchdialog.ui

RESOURCES += \
    ../icons/icons.qrc

DISTFILES += \
    studio.rc
