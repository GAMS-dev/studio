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

TEMPLATE = app

include(../tests.pri)

QT += concurrent network svg

QWEBENGINE=true
equals(QWEBENGINE, "true") {
DEFINES += QWEBENGINE
greaterThan(QT_MAJOR_VERSION, 4): QT += webenginewidgets
}

INCLUDEPATH += \
               $$SRCPATH \
               $$SRCPATH/file \
               $$SRCPATH/gdxviewer \
               $$SRCPATH/lxiviewer \
               $$SRCPATH/modeldialog \
               $$SRCPATH/option \
               $$SRCPATH/reference \
               $$SRCPATH/search \
               $$SRCPATH/syntax \
               $$SRCPATH/editors \
               $$SRCPATH/locators \
               $$SRCPATH/support \
               $$SRCPATH/help

HEADERS += $$files(*.h, true)
#           $$files($$SRCPATH/*.h, true)

SOURCES += $$SRCPATH/abstractprocess.cpp \
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
           $$SRCPATH/option/solveroptionsetting.cpp \
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
           $$SRCPATH/editors/textmapper.cpp \
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
SOURCES += $$files(*.cpp, true)

FORMS += $$files($$SRCPATH/*.ui, true)
