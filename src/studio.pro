#
# This file is part of the GAMS Studio project.
#
# Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
# Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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

# Set this to "false" to build Studio without QWebEngine enabled,
# which deactivates the studio help view.
QWEBENGINE=true

QT       += core gui svg concurrent network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = studio
TEMPLATE = app
DESTDIR = bin

CONFIG += c++14

# Setup and include the GAMS distribution
include(../gamsdependency.pri)

macx {
# ! The icns-file is created from a folder named gams.iconset containing images in multiple sizes.
# ! On mac osX type the command: iconutil -c icns [base-folder]/gams.iconset to create gams.icns
    ICON = ../icons/studio.icns

    HEADERS += ../platform/macos/macoscocoabridge.h \
               ../platform/macos/macospathfinder.h

    SOURCES += ../platform/macos/macospathfinder.cpp

    OBJECTIVE_SOURCES += ../platform/macos/macoscocoabridge.mm

    LIBS += -framework AppKit

    MACOS_BUNDLE_ICONS.files = ../icons/database.icns
    MACOS_BUNDLE_ICONS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += MACOS_BUNDLE_ICONS

    QMAKE_INFO_PLIST = ../platform/macos/info.plist
}
unix {
    LIBS += -ldl
}
win32 {
    RC_FILE += ../platform/windows/studio.rc
    LIBS += -luser32
}

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
    abstractmiroprocess.cpp \
    abstractprocess.cpp \
    application.cpp \
    autosavehandler.cpp \
    gdxdiffdialog/filepathlineedit.cpp \
    mirodeploydialog.cpp \
    mirodeployprocess.cpp \
    miromodelassemblydialog.cpp \
    miropaths.cpp \
    miroprocess.cpp \
    support/checkforupdatewrapper.cpp \
    commandlineparser.cpp \
    commonpaths.cpp \
    support/distributionvalidator.cpp \
    exception.cpp \
    file/dynamicfile.cpp \
    file/fileevent.cpp \
    file/filemeta.cpp \
    file/filemetarepo.cpp \
    file/filetype.cpp \
    file/projectabstractnode.cpp \
    file/projectcontextmenu.cpp \
    file/projectfilenode.cpp \
    file/projectgroupnode.cpp \
    file/projectlognode.cpp \
    file/projectrepo.cpp \
    file/projecttreemodel.cpp \
    file/projecttreeview.cpp \
    file/treeitemdelegate.cpp \
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
    gdxviewer/nestedheaderview.cpp \
    gdxviewer/tableviewmodel.cpp \
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
    option/definitionitemdelegate.cpp \
    option/gamsoptiondefinitionmodel.cpp \
    option/gamsoptiontablemodel.cpp \
    option/lineeditcompleteevent.cpp \
    option/option.cpp \
    option/optioncompleterdelegate.cpp \
    option/optiondefinitionitem.cpp \
    option/optiondefinitionmodel.cpp \
    option/optionsortfilterproxymodel.cpp \
    option/optiontokenizer.cpp \
    option/optionwidget.cpp \
    option/solveroptiondefinitionmodel.cpp \
    option/solveroptiontablemodel.cpp \
    option/solveroptionwidget.cpp \
    reference/reference.cpp \
    reference/referencetabstyle.cpp \
    reference/referencedatatype.cpp \
    reference/referenceitemmodel.cpp \
    reference/referencetreemodel.cpp \
    reference/referenceviewer.cpp \
    reference/symboldatatype.cpp \
    reference/symbolreferenceitem.cpp \
    reference/symbolreferencewidget.cpp \
    reference/symboltablemodel.cpp \
    search/resultsview.cpp \
    search/searchdialog.cpp \
    search/result.cpp \
    search/searchresultlist.cpp \
    search/searchlocator.cpp \
    settingsdialog.cpp \
    statuswidgets.cpp \
    studiosettings.cpp \
    syntax/basehighlighter.cpp \
    syntax/syntaxdeclaration.cpp \
    syntax/syntaxformats.cpp \
    syntax/syntaxhighlighter.cpp \
    syntax/syntaxidentifier.cpp \
    syntax/systemloghighlighter.cpp \
    syntax/textmark.cpp \
    syntax/textmarkrepo.cpp \
    support/updatedialog.cpp \
    welcomepage.cpp \
    wplabel.cpp \
    gotodialog.cpp \
    editors/abstractedit.cpp \
    editors/abstracttextmapper.cpp \
    editors/codeedit.cpp \
    editors/editorhelper.cpp \
    editors/processlogedit.cpp \
    editors/systemlogedit.cpp \
    editors/textview.cpp \
    editors/textviewedit.cpp \
    editors/viewhelper.cpp \
    encodingsdialog.cpp \
    tabdialog.cpp \
    settingslocator.cpp \
    editors/sysloglocator.cpp \
    editors/defaultsystemlogger.cpp \
    support/aboutgamsdialog.cpp         \
    support/gamslicenseinfo.cpp         \
    support/solverconfiginfo.cpp        \
    support/solvertablemodel.cpp        \
    maintabcontextmenu.cpp \
    logtabcontextmenu.cpp \
    search/searchworker.cpp \
    editors/filemapper.cpp \
    editors/memorymapper.cpp \
    editors/logparser.cpp \
    gdxdiffdialog/gdxdiffdialog.cpp \
    gdxdiffdialog/gdxdiffprocess.cpp

HEADERS += \
    abstractmiroprocess.h \
    abstractprocess.h \
    application.h \
    autosavehandler.h \
    gdxdiffdialog/filepathlineedit.h \
    mirodeploydialog.h \
    mirodeployprocess.h \
    miromodelassemblydialog.h \
    miropaths.h \
    miroprocess.h \
    support/checkforupdatewrapper.h \
    commandlineparser.h \
    common.h \
    commonpaths.h \
    support/distributionvalidator.h \
    exception.h \
    file.h \
    file/dynamicfile.h \
    file/fileevent.h \
    file/filemeta.h \
    file/filemetarepo.h \
    file/filetype.h \
    file/projectabstractnode.h \
    file/projectcontextmenu.h \
    file/projectfilenode.h \
    file/projectgroupnode.h \
    file/projectlognode.h \
    file/projectrepo.h \
    file/projecttreemodel.h \
    file/projecttreeview.h \
    file/treeitemdelegate.h \
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
    gdxviewer/nestedheaderview.h \
    gdxviewer/tableviewmodel.h \
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
    option/definitionitemdelegate.h \
    option/gamsoptiondefinitionmodel.h \
    option/gamsoptiontablemodel.h \
    option/lineeditcompleteevent.h \
    option/option.h \
    option/optioncompleterdelegate.h \
    option/optiondefinitionitem.h \
    option/optiondefinitionmodel.h \
    option/optionsortfilterproxymodel.h \
    option/optiontokenizer.h \
    option/optionwidget.h \
    option/solveroptiondefinitionmodel.h \
    option/solveroptiontablemodel.h \
    option/solveroptionwidget.h \
    reference/reference.h \
    reference/referencetabstyle.h \
    reference/referencedatatype.h \
    reference/referenceitemmodel.h \
    reference/referencetreemodel.h \
    reference/referenceviewer.h \
    reference/symboldatatype.h \
    reference/symbolreferenceitem.h \
    reference/symbolreferencewidget.h \
    reference/symboltablemodel.h \
    search/resultsview.h \
    search/searchdialog.h \
    search/result.h \
    search/searchresultlist.h \
    search/searchlocator.h \
    settingsdialog.h \
    statuswidgets.h \
    studiosettings.h \
    syntax.h \
    syntax/basehighlighter.h \
    syntax/blockcode.h \
    syntax/syntaxdeclaration.h \
    syntax/syntaxformats.h \
    syntax/syntaxhighlighter.h \
    syntax/syntaxidentifier.h \
    syntax/systemloghighlighter.h \
    syntax/textmark.h \
    syntax/textmarkrepo.h \
    support/updatedialog.h \
    version.h \
    welcomepage.h \
    wplabel.h \
    gotodialog.h \
    editors/abstractedit.h \
    editors/abstracttextmapper.h \
    editors/codeedit.h \
    editors/editorhelper.h \
    editors/processlogedit.h \
    editors/systemlogedit.h \
    editors/textview.h \
    editors/textviewedit.h \
    editors/viewhelper.h \
    encodingsdialog.h \
    tabdialog.h \
    settingslocator.h \
    editors/sysloglocator.h \
    editors/abstractsystemlogger.h \
    editors/defaultsystemlogger.h \
    support/aboutgamsdialog.h       \
    support/gamslicenseinfo.h       \
    support/solverconfiginfo.h      \
    support/solvertablemodel.h      \
    maintabcontextmenu.h \
    logtabcontextmenu.h \
    search/searchworker.h \
    editors/filemapper.h \
    editors/memorymapper.h \
    editors/logparser.h \
    gdxdiffdialog/gdxdiffdialog.h \
    gdxdiffdialog/gdxdiffprocess.h

FORMS += \
    gdxviewer/columnfilterframe.ui \
    gdxviewer/gdxsymbolview.ui \
    gdxviewer/gdxviewer.ui \
    lxiviewer/lxiviewer.ui \
    mainwindow.ui \
    mirodeploydialog.ui \
    miromodelassemblydialog.ui \
    modeldialog/modeldialog.ui \
    option/optionwidget.ui \
    option/solveroptionwidget.ui \
    reference/referenceviewer.ui \
    reference/symbolreferencewidget.ui \
    search/resultsview.ui \
    search/searchdialog.ui \
    settingsdialog.ui \
    support/updatedialog.ui \
    welcomepage.ui \
    gotodialog.ui \
    encodingsdialog.ui \
    tabdialog.ui \
    support/aboutgamsdialog.ui \
    gdxdiffdialog/gdxdiffdialog.ui

RESOURCES += \
    ../icons/icons.qrc

DISTFILES += \
    ../platform/windows/studio.rc

equals(QWEBENGINE, "true") {
DEFINES += QWEBENGINE
greaterThan(QT_MAJOR_VERSION, 4): QT += webenginewidgets
SOURCES += help/bookmarkdialog.cpp \
    help/helppage.cpp \
    help/helptoolbar.cpp \
    help/helpview.cpp \
    help/helpwidget.cpp
HEADERS += help/bookmarkdialog.h \
    help/helpdata.h \
    help/helppage.h \
    help/helptoolbar.h \
    help/helpview.h \
    help/helpwidget.h
FORMS += help/bookmarkdialog.ui \
    help/helpwidget.ui
} else {
    message("Building Studio without QWebEngine support.")
}

OTHER_FILES +=                                      \
    ../platform/macos/studio.entitlements.plist     \
    ../platform/macos/webengine.entitlements.plist  \
    ../platform/linux/gamsstudio.desktop            \
    ../CHANGELOG                                    \
    ../jenkinsfile                                  \
    ../jenkinsfile-ci                               \
    ../gamsstudio.desktop                           \
    ../CHANGELOG                                    \
    ../version
