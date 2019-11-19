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
    abstractprocess.cpp \
    application.cpp \
    autosavehandler.cpp \
    color.cpp \
    support/checkforupdatewrapper.cpp \
    commandlineparser.cpp \
    commonpaths.cpp \
    editors/abstractedit.cpp \
    editors/abstracttextmapper.cpp \
    editors/codeedit.cpp \
    editors/defaultsystemlogger.cpp \
    editors/editorhelper.cpp \
    editors/filemapper.cpp \
    editors/logparser.cpp \
    editors/memorymapper.cpp \
    editors/processlogedit.cpp \
    editors/sysloglocator.cpp \
    editors/systemlogedit.cpp \
    editors/textview.cpp \
    editors/textviewedit.cpp \
    editors/viewhelper.cpp \
    encodingsdialog.cpp \
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
    gdxdiffdialog/filepathlineedit.cpp \
    gdxdiffdialog/gdxdiffdialog.cpp \
    gdxdiffdialog/gdxdiffprocess.cpp \
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
    gotodialog.cpp \
    keys.cpp \
    logger.cpp \
    logtabcontextmenu.cpp \
    lxiviewer/lxiparser.cpp \
    lxiviewer/lxitreeitem.cpp \
    lxiviewer/lxitreemodel.cpp \
    lxiviewer/lxiviewer.cpp \
    main.cpp \
    maintabcontextmenu.cpp \
    mainwindow.cpp \
    miro/abstractmiroprocess.cpp \
    miro/mirocommon.cpp \
    miro/mirodeploydialog.cpp \
    miro/mirodeployprocess.cpp \
    miro/miromodelassemblydialog.cpp \
    miro/miroprocess.cpp \
    modeldialog/glbparser.cpp   \
    modeldialog/library.cpp     \
    modeldialog/libraryitem.cpp \
    modeldialog/librarymodel.cpp \
    modeldialog/modeldialog.cpp \
    option/addoptionheaderview.cpp \
    option/commandline.cpp \
    option/definitionitemdelegate.cpp \
    option/gamsoptiondefinitionmodel.cpp \
    option/gamsparametertablemodel.cpp \
    option/lineeditcompleteevent.cpp \
    option/option.cpp \
    option/optioncompleterdelegate.cpp \
    option/optiondefinitionitem.cpp \
    option/optiondefinitionmodel.cpp \
    option/optionsortfilterproxymodel.cpp \
    option/optiontokenizer.cpp \
    option/parametereditor.cpp \
    option/solveroptiondefinitionmodel.cpp \
    option/solveroptiontablemodel.cpp \
    option/solveroptionwidget.cpp \
    reference/reference.cpp \
    reference/referencedatatype.cpp \
    reference/referenceitemmodel.cpp \
    reference/referencetabstyle.cpp \
    reference/referencetreemodel.cpp \
    reference/referenceviewer.cpp \
    reference/symboldatatype.cpp \
    reference/symbolreferenceitem.cpp \
    reference/symbolreferencewidget.cpp \
    reference/symboltablemodel.cpp \
    search/result.cpp \
    search/resultsview.cpp \
    search/searchdialog.cpp \
    search/searchlocator.cpp \
    search/searchresultlist.cpp \
    search/searchworker.cpp \
    settingsdialog.cpp \
    settingslocator.cpp \
    statuswidgets.cpp \
    studiosettings.cpp \
    support/aboutgamsdialog.cpp         \
    support/checkforupdatewrapper.cpp \
    support/distributionvalidator.cpp \
    support/gamslicenseinfo.cpp         \
    support/solverconfiginfo.cpp        \
    support/solvertablemodel.cpp        \
    support/updatedialog.cpp \
    syntax/basehighlighter.cpp \
    syntax/syntaxdeclaration.cpp \
    syntax/syntaxformats.cpp \
    syntax/syntaxhighlighter.cpp \
    syntax/syntaxidentifier.cpp \
    syntax/systemloghighlighter.cpp \
    syntax/textmark.cpp \
    syntax/textmarkrepo.cpp \
    tabdialog.cpp \
    welcomepage.cpp \
    wplabel.cpp

HEADERS += \
    abstractprocess.h \
    application.h \
    autosavehandler.h \
    color.h \
    support/checkforupdatewrapper.h \
    commandlineparser.h \
    common.h \
    commonpaths.h \
    editors/abstractedit.h \
    editors/abstractsystemlogger.h \
    editors/abstracttextmapper.h \
    editors/codeedit.h \
    editors/defaultsystemlogger.h \
    editors/editorhelper.h \
    editors/filemapper.h \
    editors/logparser.h \
    editors/memorymapper.h \
    editors/processlogedit.h \
    editors/sysloglocator.h \
    editors/systemlogedit.h \
    editors/textview.h \
    editors/textviewedit.h \
    editors/viewhelper.h \
    encodingsdialog.h \
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
    gdxdiffdialog/filepathlineedit.h \
    gdxdiffdialog/gdxdiffdialog.h \
    gdxdiffdialog/gdxdiffprocess.h \
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
    gotodialog.h \
    keys.h \
    logger.h \
    logtabcontextmenu.h \
    lxiviewer/lxiparser.h \
    lxiviewer/lxitreeitem.h \
    lxiviewer/lxitreemodel.h \
    lxiviewer/lxiviewer.h \
    maintabcontextmenu.h \
    mainwindow.h \
    miro/abstractmiroprocess.h \
    miro/mirocommon.h \
    miro/mirodeploydialog.h \
    miro/mirodeployprocess.h \
    miro/miromodelassemblydialog.h \
    miro/miroprocess.h \
    modeldialog/glbparser.h \
    modeldialog/library.h \
    modeldialog/libraryitem.h \
    modeldialog/librarymodel.h \
    modeldialog/modeldialog.h \
    option/addoptionheaderview.h \
    option/commandline.h \
    option/definitionitemdelegate.h \
    option/gamsoptiondefinitionmodel.h \
    option/gamsparametertablemodel.h \
    option/lineeditcompleteevent.h \
    option/option.h \
    option/optioncompleterdelegate.h \
    option/optiondefinitionitem.h \
    option/optiondefinitionmodel.h \
    option/optionsortfilterproxymodel.h \
    option/optiontokenizer.h \
    option/parametereditor.h \
    option/solveroptiondefinitionmodel.h \
    option/solveroptiontablemodel.h \
    option/solveroptionwidget.h \
    reference/reference.h \
    reference/referencedatatype.h \
    reference/referenceitemmodel.h \
    reference/referencetabstyle.h \
    reference/referencetreemodel.h \
    reference/referenceviewer.h \
    reference/symboldatatype.h \
    reference/symbolreferenceitem.h \
    reference/symbolreferencewidget.h \
    reference/symboltablemodel.h \
    search/result.h \
    search/resultsview.h \
    search/searchdialog.h \
    search/searchlocator.h \
    search/searchresultlist.h \
    search/searchworker.h \
    settingsdialog.h \
    settingslocator.h \
    statuswidgets.h \
    studiosettings.h \
    support/aboutgamsdialog.h       \
    support/checkforupdatewrapper.h \
    support/distributionvalidator.h \
    support/gamslicenseinfo.h       \
    support/solverconfiginfo.h      \
    support/solvertablemodel.h      \
    support/updatedialog.h \
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
    tabdialog.h \
    version.h \
    welcomepage.h \
    wplabel.h

FORMS += \
    encodingsdialog.ui \
    gdxdiffdialog/gdxdiffdialog.ui \
    gdxviewer/columnfilterframe.ui \
    gdxviewer/gdxsymbolview.ui \
    gdxviewer/gdxviewer.ui \
    gotodialog.ui \
    lxiviewer/lxiviewer.ui \
    mainwindow.ui \
    miro/mirodeploydialog.ui \
    miro/miromodelassemblydialog.ui \
    modeldialog/modeldialog.ui \
    option/parametereditor.ui \
    option/solveroptionwidget.ui \
    reference/referenceviewer.ui \
    reference/symbolreferencewidget.ui \
    search/resultsview.ui \
    search/searchdialog.ui \
    settingsdialog.ui \
    support/aboutgamsdialog.ui \
    support/updatedialog.ui \
    tabdialog.ui \
    welcomepage.ui

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
