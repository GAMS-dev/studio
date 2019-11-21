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

TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

HEADERS += $$files(*.h, true) \
           $$SRCPATH/abstractprocess.h \
           $$SRCPATH/application.h \
           $$SRCPATH/autosavehandler.h \
           $$SRCPATH/support/checkforupdatewrapper.h \
           $$SRCPATH/commandlineparser.h \
           $$SRCPATH/common.h \
           $$SRCPATH/commonpaths.h \
           $$SRCPATH/support/distributionvalidator.h \
           $$SRCPATH/exception.h \
           $$SRCPATH/file.h \
           $$SRCPATH/file/dynamicfile.h \
           $$SRCPATH/file/fileevent.h \
           $$SRCPATH/file/filemeta.h \
           $$SRCPATH/file/filemetarepo.h \
           $$SRCPATH/file/filetype.h \
           $$SRCPATH/file/projectabstractnode.h \
           $$SRCPATH/file/projectcontextmenu.h \
           $$SRCPATH/file/projectfilenode.h \
           $$SRCPATH/file/projectgroupnode.h \
           $$SRCPATH/file/projectlognode.h \
           $$SRCPATH/file/projectrepo.h \
           $$SRCPATH/file/projecttreemodel.h \
           $$SRCPATH/file/projecttreeview.h \
           $$SRCPATH/file/treeitemdelegate.h \
           $$SRCPATH/gamslibprocess.h \
           $$SRCPATH/gamsprocess.h \
           $$SRCPATH/gdxdiffdialog/filepathlineedit.h \
           $$SRCPATH/gdxdiffdialog/gdxdiffdialog.h \
           $$SRCPATH/gdxdiffdialog/gdxdiffprocess.h \
           $$SRCPATH/gdxviewer/columnfilter.h \
           $$SRCPATH/gdxviewer/columnfilterframe.h \
           $$SRCPATH/gdxviewer/filteruelmodel.h \
           $$SRCPATH/gdxviewer/gdxsymbol.h \
           $$SRCPATH/gdxviewer/gdxsymbolheaderview.h \
           $$SRCPATH/gdxviewer/gdxsymboltable.h \
           $$SRCPATH/gdxviewer/gdxsymbolview.h \
           $$SRCPATH/gdxviewer/gdxviewer.h \
           $$SRCPATH/gdxviewer/nestedheaderview.h \
           $$SRCPATH/gdxviewer/tableviewmodel.h \
           $$SRCPATH/keys.h \
           $$SRCPATH/locators/searchlocator.h \
           $$SRCPATH/logger.h \
           $$SRCPATH/lxiviewer/lxiparser.h \
           $$SRCPATH/lxiviewer/lxitreeitem.h \
           $$SRCPATH/lxiviewer/lxitreemodel.h \
           $$SRCPATH/lxiviewer/lxiviewer.h \
           $$SRCPATH/mainwindow.h \
           $$SRCPATH/modeldialog/glbparser.h \
           $$SRCPATH/modeldialog/library.h \
           $$SRCPATH/modeldialog/libraryitem.h \
           $$SRCPATH/modeldialog/librarymodel.h \
           $$SRCPATH/modeldialog/modeldialog.h \
           $$SRCPATH/option/addoptionheaderview.h \
           $$SRCPATH/option/commandlineoption.h \
           $$SRCPATH/option/definitionitemdelegate.h \
           $$SRCPATH/option/gamsoptiondefinitionmodel.h \
           $$SRCPATH/option/gamsoptiontablemodel.h \
           $$SRCPATH/option/lineeditcompleteevent.h \
           $$SRCPATH/option/option.h \
           $$SRCPATH/option/optioncompleterdelegate.h \
           $$SRCPATH/option/optiondefinitionitem.h \
           $$SRCPATH/option/optiondefinitionmodel.h \
           $$SRCPATH/option/optionsortfilterproxymodel.h \
           $$SRCPATH/option/optiontokenizer.h \
           $$SRCPATH/option/optionwidget.h \
           $$SRCPATH/option/solveroptiondefinitionmodel.h \
           $$SRCPATH/option/solveroptiontablemodel.h \
           $$SRCPATH/option/solveroptionwidget.h \
           $$SRCPATH/reference/reference.h \
           $$SRCPATH/reference/referencetabstyle.h \
           $$SRCPATH/reference/referencedatatype.h \
           $$SRCPATH/reference/referenceitemmodel.h \
           $$SRCPATH/reference/referencetreemodel.h \
           $$SRCPATH/reference/referenceviewer.h \
           $$SRCPATH/reference/symboldatatype.h \
           $$SRCPATH/reference/symbolreferenceitem.h \
           $$SRCPATH/reference/symbolreferencewidget.h \
           $$SRCPATH/reference/symboltablemodel.h \
           $$SRCPATH/resultsview.h \
           $$SRCPATH/search/searchdialog.h \
           $$SRCPATH/search/result.h \
           $$SRCPATH/search/searchresultlist.h \
           $$SRCPATH/settingsdialog.h \
           $$SRCPATH/statuswidgets.h \
           $$SRCPATH/studiosettings.h \
           $$SRCPATH/support/solverconfiginfo.h \
           $$SRCPATH/syntax.h \
           $$SRCPATH/syntax/basehighlighter.h \
           $$SRCPATH/syntax/blockcode.h \
           $$SRCPATH/syntax/syntaxdeclaration.h \
           $$SRCPATH/syntax/syntaxformats.h \
           $$SRCPATH/syntax/syntaxhighlighter.h \
           $$SRCPATH/syntax/syntaxidentifier.h \
           $$SRCPATH/syntax/systemloghighlighter.h \
           $$SRCPATH/syntax/textmark.h \
           $$SRCPATH/syntax/textmarkrepo.h \
           $$SRCPATH/support/updatedialog.h \
           $$SRCPATH/version.h \
           $$SRCPATH/welcomepage.h \
           $$SRCPATH/wplabel.h \
           $$SRCPATH/gotodialog.h \
           $$SRCPATH/editors/abstractedit.h \
           $$SRCPATH/editors/codeedit.h \
           $$SRCPATH/editors/editorhelper.h \
           $$SRCPATH/editors/processlogedit.h \
           $$SRCPATH/editors/systemlogedit.h \
           $$SRCPATH/editors/abstracttextmapper.h \
           $$SRCPATH/editors/logparser.h \
           $$SRCPATH/editors/filemapper.h \
           $$SRCPATH/editors/memorymapper.h \
           $$SRCPATH/editors/textview.h \
           $$SRCPATH/editors/textviewedit.h \
           $$SRCPATH/editors/viewhelper.h \
           $$SRCPATH/encodingsdialog.h \
           $$SRCPATH/tabdialog.h \
           $$SRCPATH/locators/settingslocator.h \
           $$SRCPATH/locators/sysloglocator.h \
           $$SRCPATH/locators/abstractsystemlogger.h \
           $$SRCPATH/locators/defaultsystemlogger.h \
           $$SRCPATH/support/aboutgamsdialog.h       \
           $$SRCPATH/support/gamslicenseinfo.h       \
           $$SRCPATH/support/solvertablemodel.h      \
           $$SRCPATH/maintabcontextmenu.h \
           $$SRCPATH/logtabcontextmenu.h \
           $$SRCPATH/search/searchworker.h

SOURCES += $$files(*.cpp, true) \
           $$SRCPATH/abstractprocess.cpp \
           $$SRCPATH/application.cpp \
           $$SRCPATH/autosavehandler.cpp \
           $$SRCPATH/support/checkforupdatewrapper.cpp \
           $$SRCPATH/commandlineparser.cpp \
           $$SRCPATH/commonpaths.cpp \
           $$SRCPATH/support/distributionvalidator.cpp \
           $$SRCPATH/exception.cpp \
           $$SRCPATH/file/dynamicfile.cpp \
           $$SRCPATH/file/fileevent.cpp \
           $$SRCPATH/file/filemeta.cpp \
           $$SRCPATH/file/filemetarepo.cpp \
           $$SRCPATH/file/filetype.cpp \
           $$SRCPATH/file/projectabstractnode.cpp \
           $$SRCPATH/file/projectcontextmenu.cpp \
           $$SRCPATH/file/projectfilenode.cpp \
           $$SRCPATH/file/projectgroupnode.cpp \
           $$SRCPATH/file/projectlognode.cpp \
           $$SRCPATH/file/projectrepo.cpp \
           $$SRCPATH/file/projecttreemodel.cpp \
           $$SRCPATH/file/projecttreeview.cpp \
           $$SRCPATH/file/treeitemdelegate.cpp \
           $$SRCPATH/gamslibprocess.cpp  \
           $$SRCPATH/gamsprocess.cpp     \
           $$SRCPATH/gdxdiffdialog/filepathlineedit.cpp \
           $$SRCPATH/gdxdiffdialog/gdxdiffdialog.cpp \
           $$SRCPATH/gdxdiffdialog/gdxdiffprocess.cpp \
           $$SRCPATH/gdxviewer/columnfilter.cpp \
           $$SRCPATH/gdxviewer/columnfilterframe.cpp \
           $$SRCPATH/gdxviewer/filteruelmodel.cpp \
           $$SRCPATH/gdxviewer/gdxsymbol.cpp \
           $$SRCPATH/gdxviewer/gdxsymbolheaderview.cpp \
           $$SRCPATH/gdxviewer/gdxsymboltable.cpp \
           $$SRCPATH/gdxviewer/gdxsymbolview.cpp \
           $$SRCPATH/gdxviewer/gdxviewer.cpp \
           $$SRCPATH/gdxviewer/nestedheaderview.cpp \
           $$SRCPATH/gdxviewer/tableviewmodel.cpp \
           $$SRCPATH/keys.cpp \
           $$SRCPATH/locators/searchlocator.cpp \
           $$SRCPATH/logger.cpp \
           $$SRCPATH/lxiviewer/lxiparser.cpp \
           $$SRCPATH/lxiviewer/lxitreeitem.cpp \
           $$SRCPATH/lxiviewer/lxitreemodel.cpp \
           $$SRCPATH/lxiviewer/lxiviewer.cpp \
           $$SRCPATH/mainwindow.cpp \
           $$SRCPATH/modeldialog/glbparser.cpp   \
           $$SRCPATH/modeldialog/library.cpp     \
           $$SRCPATH/modeldialog/libraryitem.cpp \
           $$SRCPATH/modeldialog/librarymodel.cpp \
           $$SRCPATH/modeldialog/modeldialog.cpp \
           $$SRCPATH/option/addoptionheaderview.cpp \
           $$SRCPATH/option/commandlineoption.cpp \
           $$SRCPATH/option/definitionitemdelegate.cpp \
           $$SRCPATH/option/gamsoptiondefinitionmodel.cpp \
           $$SRCPATH/option/gamsoptiontablemodel.cpp \
           $$SRCPATH/option/lineeditcompleteevent.cpp \
           $$SRCPATH/option/option.cpp \
           $$SRCPATH/option/optioncompleterdelegate.cpp \
           $$SRCPATH/option/optiondefinitionitem.cpp \
           $$SRCPATH/option/optiondefinitionmodel.cpp \
           $$SRCPATH/option/optionsortfilterproxymodel.cpp \
           $$SRCPATH/option/optiontokenizer.cpp \
           $$SRCPATH/option/optionwidget.cpp \
           $$SRCPATH/option/solveroptiondefinitionmodel.cpp \
           $$SRCPATH/option/solveroptiontablemodel.cpp \
           $$SRCPATH/option/solveroptionwidget.cpp \
           $$SRCPATH/reference/reference.cpp \
           $$SRCPATH/reference/referencetabstyle.cpp \
           $$SRCPATH/reference/referencedatatype.cpp \
           $$SRCPATH/reference/referenceitemmodel.cpp \
           $$SRCPATH/reference/referencetreemodel.cpp \
           $$SRCPATH/reference/referenceviewer.cpp \
           $$SRCPATH/reference/symboldatatype.cpp \
           $$SRCPATH/reference/symbolreferenceitem.cpp \
           $$SRCPATH/reference/symbolreferencewidget.cpp \
           $$SRCPATH/reference/symboltablemodel.cpp \
           $$SRCPATH/resultsview.cpp \
           $$SRCPATH/search/searchdialog.cpp \
           $$SRCPATH/search/result.cpp \
           $$SRCPATH/search/searchresultlist.cpp \
           $$SRCPATH/settingsdialog.cpp \
           $$SRCPATH/statuswidgets.cpp \
           $$SRCPATH/studiosettings.cpp \
           $$SRCPATH/support/solverconfiginfo.cpp \
           $$SRCPATH/syntax/basehighlighter.cpp \
           $$SRCPATH/syntax/syntaxdeclaration.cpp \
           $$SRCPATH/syntax/syntaxformats.cpp \
           $$SRCPATH/syntax/syntaxhighlighter.cpp \
           $$SRCPATH/syntax/syntaxidentifier.cpp \
           $$SRCPATH/syntax/systemloghighlighter.cpp \
           $$SRCPATH/syntax/textmark.cpp \
           $$SRCPATH/syntax/textmarkrepo.cpp \
           $$SRCPATH/support/updatedialog.cpp \
           $$SRCPATH/welcomepage.cpp \
           $$SRCPATH/wplabel.cpp \
           $$SRCPATH/gotodialog.cpp \
           $$SRCPATH/editors/abstractedit.cpp \
           $$SRCPATH/editors/codeedit.cpp \
           $$SRCPATH/editors/editorhelper.cpp \
           $$SRCPATH/editors/processlogedit.cpp \
           $$SRCPATH/editors/systemlogedit.cpp \
           $$SRCPATH/editors/abstracttextmapper.cpp \
           $$SRCPATH/editors/logparser.cpp \
           $$SRCPATH/editors/filemapper.cpp \
           $$SRCPATH/editors/memorymapper.cpp \
           $$SRCPATH/editors/textview.cpp \
           $$SRCPATH/editors/textviewedit.cpp \
           $$SRCPATH/editors/viewhelper.cpp \
           $$SRCPATH/encodingsdialog.cpp \
           $$SRCPATH/tabdialog.cpp \
           $$SRCPATH/locators/settingslocator.cpp \
           $$SRCPATH/locators/sysloglocator.cpp \
           $$SRCPATH/locators/defaultsystemlogger.cpp \
           $$SRCPATH/support/aboutgamsdialog.cpp         \
           $$SRCPATH/support/gamslicenseinfo.cpp         \
           $$SRCPATH/support/solvertablemodel.cpp        \
           $$SRCPATH/maintabcontextmenu.cpp \
           $$SRCPATH/logtabcontextmenu.cpp \
           $$SRCPATH/search/searchworker.cpp

FORMS += $$SRCPATH/gdxdiffdialog/gdxdiffdialog.ui \
         $$SRCPATH/gdxviewer/columnfilterframe.ui \
         $$SRCPATH/gdxviewer/gdxsymbolview.ui \
         $$SRCPATH/gdxviewer/gdxviewer.ui \
         $$SRCPATH/lxiviewer/lxiviewer.ui \
         $$SRCPATH/mainwindow.ui \
         $$SRCPATH/modeldialog/modeldialog.ui \
         $$SRCPATH/option/optionwidget.ui \
         $$SRCPATH/option/solveroptionwidget.ui \
         $$SRCPATH/reference/referenceviewer.ui \
         $$SRCPATH/reference/symbolreferencewidget.ui \
         $$SRCPATH/resultsview.ui \
         $$SRCPATH/search/searchdialog.ui \
         $$SRCPATH/settingsdialog.ui \
         $$SRCPATH/support/updatedialog.ui \
         $$SRCPATH/welcomepage.ui \
         $$SRCPATH/gotodialog.ui \
         $$SRCPATH/encodingsdialog.ui \
         $$SRCPATH/tabdialog.ui \
         $$SRCPATH/support/aboutgamsdialog.ui

QWEBENGINE=true
equals(QWEBENGINE, "true") {
DEFINES += QWEBENGINE
greaterThan(QT_MAJOR_VERSION, 4): QT += webenginewidgets
SOURCES += $$SRCPATH/help/bookmarkdialog.cpp \
           $$SRCPATH/help/helppage.cpp \
           $$SRCPATH/help/helptoolbar.cpp \
           $$SRCPATH/help/helpview.cpp \
           $$SRCPATH/help/helpwidget.cpp

HEADERS += $$SRCPATH/help/bookmarkdialog.h \
           $$SRCPATH/help/helpdata.h \
           $$SRCPATH/help/helppage.h \
           $$SRCPATH/help/helptoolbar.h \
           $$SRCPATH/help/helpview.h \
           $$SRCPATH/help/helpwidget.h

FORMS += $$SRCPATH/help/bookmarkdialog.ui \
         $$SRCPATH/help/helpwidget.ui
} else {
    message("Building Studio without QWebEngine support.")
}
