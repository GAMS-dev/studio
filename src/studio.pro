QT += core widgets gui svg concurrent network printsupport core5compat

TARGET = studio
TEMPLATE = app
DESTDIR = bin
CONFIG += c++17

# Setup and include the GAMS distribution
include(../gamsdependency.pri)

INCLUDEPATH += $$PWD/../extern
include(../extern/dtoaloc/dtoaloc.pri)
include(../extern/engineapi/engineapi.pri)
include(../extern/yaml-cpp/yaml-cpp.pri)

OBJECTS_DIR=../objects
MOC_DIR=../objects

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
    LIBS += -ldl -lpthread
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
    abstractview.cpp \
    application.cpp \
    autosavehandler.cpp \
    colors/palettemanager.cpp \
    commandlineparser.cpp \
    common.cpp \
    commonpaths.cpp \
    confirmdialog.cpp \
    connect/clickablelabel.cpp \
    connect/connect.cpp \
    connect/connectagent.cpp \
    connect/connectdata.cpp \
    connect/connectdataactiondelegate.cpp \
    connect/connectdataitem.cpp \
    connect/connectdatakeydelegate.cpp \
    connect/connectdatamodel.cpp \
    connect/connectdatavaluedelegate.cpp \
    connect/connecteditor.cpp \
    connect/connecterror.cpp \
    connect/connectschema.cpp \
    connect/schemadefinitionitem.cpp \
    connect/schemadefinitionmodel.cpp \
    connect/schemalistmodel.cpp \
    connect/treecellresizer.cpp \
    debugger/breakpointdata.cpp \
    debugger/debugwidget.cpp \
    debugger/pincontrol.cpp \
    debugger/server.cpp \
    editors/abstractedit.cpp \
    editors/abstracttextmapper.cpp \
    editors/chunktextmapper.cpp \
    editors/codecompleter.cpp \
    editors/codeedit.cpp \
    editors/defaultsystemlogger.cpp \
    editors/editorhelper.cpp \
    editors/fastfilemapper.cpp \
    editors/filemapper.cpp \
    editors/logparser.cpp \
    editors/memorymapper.cpp \
    editors/navigationhistory.cpp \
    editors/navigationhistorylocator.cpp \
    editors/sysloglocator.cpp \
    editors/systemlogedit.cpp \
    editors/textview.cpp \
    editors/textviewedit.cpp \
    encoding.cpp \
    encodingsdialog.cpp \
    engine/authmanager.cpp \
    engine/efieditor.cpp \
    engine/enginemanager.cpp \
    engine/engineprocess.cpp \
    engine/enginestartdialog.cpp \
    exception.cpp \
    file/dynamicfile.cpp \
    file/filechangedialog.cpp \
    file/fileevent.cpp \
    file/fileicon.cpp \
    file/filemeta.cpp \
    file/filemetarepo.cpp \
    file/filetype.cpp \
    file/pathrequest.cpp \
    file/pathselect.cpp \
    file/pexabstractnode.cpp \
    file/pexfilenode.cpp \
    file/pexgroupnode.cpp \
    file/pexlognode.cpp \
    file/projectcontextmenu.cpp \
    file/projectedit.cpp \
    file/projectrepo.cpp \
    file/projecttreemodel.cpp \
    file/projecttreeview.cpp \
    file/recentdata.cpp \
    file/textfilesaver.cpp \
    file/treeitemdelegate.cpp \
    fileeventhandler.cpp \
    filesystemmodel.cpp \
    filesystemwidget.cpp \
    filterlineedit.cpp \
    gdxdiffdialog/filepathlineedit.cpp \
    gdxdiffdialog/gdxdiffdialog.cpp \
    gdxdiffdialog/gdxdiffprocess.cpp \
    gdxviewer/columnfilter.cpp \
    gdxviewer/columnfilterframe.cpp \
    gdxviewer/exportdialog.cpp \
    gdxviewer/exportdriver.cpp \
    gdxviewer/exportmodel.cpp \
    gdxviewer/filteruelmodel.cpp \
    gdxviewer/gdxsymbol.cpp \
    gdxviewer/gdxsymbolheaderview.cpp \
    gdxviewer/gdxsymboltablemodel.cpp \
    gdxviewer/gdxsymbolview.cpp \
    gdxviewer/gdxsymbolviewstate.cpp \
    gdxviewer/gdxviewer.cpp \
    gdxviewer/gdxviewerstate.cpp \
    gdxviewer/nestedheaderview.cpp \
    gdxviewer/numericalformatcontroller.cpp \
    gdxviewer/quickselectlistview.cpp \
    gdxviewer/tabenabledmenu.cpp \
    gdxviewer/tableviewdomainmodel.cpp \
    gdxviewer/tableviewmodel.cpp \
    gdxviewer/valuefilter.cpp \
    gdxviewer/valuefilterwidget.cpp \
    headerviewproxy.cpp \
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
    miro/miroprocess.cpp \
    modeldialog/descriptiondialog.cpp \
    modeldialog/glbparser.cpp   \
    modeldialog/library.cpp     \
    modeldialog/libraryitem.cpp \
    modeldialog/librarymodel.cpp \
    modeldialog/modeldialog.cpp \
    modeldialog/tableview.cpp \
    msgbox.cpp \
    navigator/navigatorcontent.cpp \
    navigator/navigatordialog.cpp \
    navigator/navigatorlineedit.cpp \
    navigator/navigatormodel.cpp \
    neos/httpmanager.cpp \
    neos/neosmanager.cpp \
    neos/neosprocess.cpp \
    neos/neosstartdialog.cpp \
    neos/xmlrpc.cpp \
    networkmanager.cpp \
    numerics/doubleFormat.c \
    numerics/doubleformatter.cpp \
    option/addoptionheaderview.cpp \
    option/commandline.cpp \
    option/configoptiondefinitionmodel.cpp \
    option/configparamtablemodel.cpp \
    option/definitionitemdelegate.cpp \
    option/envvarcfgcompleterdelegate.cpp \
    option/envvarconfigeditor.cpp \
    option/envvartablemodel.cpp \
    option/gamsconfigeditor.cpp \
    option/gamsoptiondefinitionmodel.cpp \
    option/gamsparametertablemodel.cpp \
    option/gamsuserconfig.cpp \
    option/lineeditcompleteevent.cpp \
    option/option.cpp \
    option/optioncompleterdelegate.cpp \
    option/optiondefinitionitem.cpp \
    option/optiondefinitionmodel.cpp \
    option/optionsortfilterproxymodel.cpp \
    option/optiontokenizer.cpp \
    option/paramconfigeditor.cpp \
    option/parametereditor.cpp \
    option/solveroptiondefinitionmodel.cpp \
    option/solveroptiontablemodel.cpp \
    option/solveroptionwidget.cpp \
    pinviewwidget.cpp \
    process/abstractprocess.cpp \
    process/connectprocess.cpp \
    process/gamsgetkeyprocess.cpp \
    process/gamslibprocess.cpp  \
    process/gamsprocess.cpp     \
    process/gmsunzipprocess.cpp \
    process/gmszipprocess.cpp \
    reference/filereferenceitem.cpp \
    reference/filereferencewidget.cpp \
    reference/fileusedtreemodel.cpp \
    reference/reference.cpp \
    reference/referencedatatype.cpp \
    reference/referenceitemmodel.cpp \
    reference/referencetabstyle.cpp \
    reference/referencetreemodel.cpp \
    reference/referenceviewer.cpp \
    reference/sortedfileheaderview.cpp \
    reference/symboldatatype.cpp \
    reference/symbolreferenceitem.cpp \
    reference/symbolreferencewidget.cpp \
    reference/symboltablemodel.cpp \
    search/fileworker.cpp \
    search/result.cpp \
    search/resultsview.cpp \
    search/search.cpp \
    search/searchdialog.cpp \
    search/searchfilehandler.cpp \
    search/searchlocator.cpp \
    search/searchresultmodel.cpp \
    search/searchresultviewitemdelegate.cpp \
    search/searchworker.cpp \
    settings.cpp \
    settingsdialog.cpp \
    statuswidgets.cpp \
    support/checkforupdate.cpp \
    support/distributionvalidator.cpp \
    support/gamslicenseinfo.cpp         \
    support/gamslicensingdialog.cpp \
    support/solverconfiginfo.cpp        \
    support/updatewidget.cpp \
    support/versioninfoloader.cpp \
    svgengine.cpp \
    syntax/basehighlighter.cpp \
    syntax/blockdata.cpp \
    syntax/htmlconverter.cpp \
    syntax/syntaxdeclaration.cpp \
    syntax/syntaxformats.cpp \
    syntax/syntaxhighlighter.cpp \
    syntax/syntaxidentifier.cpp \
    syntax/systemloghighlighter.cpp \
    syntax/textmark.cpp \
    syntax/textmarkrepo.cpp \
    tabbarstyle.cpp \
    tabdialog.cpp \
    tabwidget.cpp \
    theme.cpp \
    themewidget.cpp \
    viewhelper.cpp \
    welcomepage.cpp \
    wplabel.cpp

HEADERS += \
    abstractview.h \
    application.h \
    autosavehandler.h \
    colors/palettemanager.h \
    commandlineparser.h \
    common.h \
    commonpaths.h \
    confirmdialog.h \
    connect/clickablelabel.h \
    connect/connect.h \
    connect/connectagent.h \
    connect/connectdata.h \
    connect/connectdataactiondelegate.h \
    connect/connectdataitem.h \
    connect/connectdatakeydelegate.h \
    connect/connectdatamodel.h \
    connect/connectdatavaluedelegate.h \
    connect/connecteditor.h \
    connect/connecterror.h \
    connect/connectschema.h \
    connect/schemadefinitionitem.h \
    connect/schemadefinitionmodel.h \
    connect/schemalistmodel.h \
    connect/treecellresizer.h \
    debugger/breakpointdata.h \
    debugger/debugwidget.h \
    debugger/pincontrol.h \
    debugger/server.h \
    editors/abstractedit.h \
    editors/abstractsystemlogger.h \
    editors/abstracttextmapper.h \
    editors/chunktextmapper.h \
    editors/codecompleter.h \
    editors/codeedit.h \
    editors/defaultsystemlogger.h \
    editors/editorhelper.h \
    editors/fastfilemapper.h \
    editors/filemapper.h \
    editors/logparser.h \
    editors/memorymapper.h \
    editors/navigationhistory.h \
    editors/navigationhistorylocator.h \
    editors/sysloglocator.h \
    editors/systemlogedit.h \
    editors/textview.h \
    editors/textviewedit.h \
    encoding.h \
    encodingsdialog.h \
    engine/authmanager.h \
    engine/efieditor.h \
    engine/enginemanager.h \
    engine/engineprocess.h \
    engine/enginestartdialog.h \
    exception.h \
    file.h \
    file/dynamicfile.h \
    file/filechangedialog.h \
    file/fileevent.h \
    file/fileicon.h \
    file/filemeta.h \
    file/filemetarepo.h \
    file/filetype.h \
    file/pathrequest.h \
    file/pathselect.h \
    file/pexabstractnode.h \
    file/pexfilenode.h \
    file/pexgroupnode.h \
    file/pexlognode.h \
    file/projectcontextmenu.h \
    file/projectedit.h \
    file/projectrepo.h \
    file/projecttreemodel.h \
    file/projecttreeview.h \
    file/recentdata.h \
    file/textfilesaver.h \
    file/treeitemdelegate.h \
    fileeventhandler.h \
    filesystemmodel.h \
    filesystemwidget.h \
    filterlineedit.h \
    gdxdiffdialog/filepathlineedit.h \
    gdxdiffdialog/gdxdiffdialog.h \
    gdxdiffdialog/gdxdiffprocess.h \
    gdxviewer/columnfilter.h \
    gdxviewer/columnfilterframe.h \
    gdxviewer/exportdialog.h \
    gdxviewer/exportdriver.h \
    gdxviewer/exportmodel.h \
    gdxviewer/filteruelmodel.h \
    gdxviewer/gdxsymbol.h \
    gdxviewer/gdxsymbolheaderview.h \
    gdxviewer/gdxsymboltablemodel.h \
    gdxviewer/gdxsymbolview.h \
    gdxviewer/gdxsymbolviewstate.h \
    gdxviewer/gdxviewer.h \
    gdxviewer/gdxviewerstate.h \
    gdxviewer/nestedheaderview.h \
    gdxviewer/numericalformatcontroller.h \
    gdxviewer/quickselectlistview.h \
    gdxviewer/tabenabledmenu.h \
    gdxviewer/tableviewdomainmodel.h \
    gdxviewer/tableviewmodel.h \
    gdxviewer/valuefilter.h \
    gdxviewer/valuefilterwidget.h \
    headerviewproxy.h \
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
    miro/miroprocess.h \
    modeldialog/descriptiondialog.h \
    modeldialog/glbparser.h \
    modeldialog/library.h \
    modeldialog/libraryitem.h \
    modeldialog/librarymodel.h \
    modeldialog/modeldialog.h \
    modeldialog/tableview.h \
    msgbox.h \
    navigator/navigatorcontent.h \
    navigator/navigatordialog.h \
    navigator/navigatorlineedit.h \
    navigator/navigatormodel.h \
    neos/httpmanager.h \
    neos/neosmanager.h \
    neos/neosprocess.h \
    neos/neosstartdialog.h \
    neos/xmlrpc.h \
    networkmanager.h \
    numerics/doubleFormat.h \
    numerics/doubleformatter.h \
    option/addoptionheaderview.h \
    option/commandline.h \
    option/configoptiondefinitionmodel.h \
    option/configparamtablemodel.h \
    option/definitionitemdelegate.h \
    option/envvarcfgcompleterdelegate.h \
    option/envvarconfigeditor.h \
    option/envvartablemodel.h \
    option/gamsconfigeditor.h \
    option/gamsoptiondefinitionmodel.h \
    option/gamsparametertablemodel.h \
    option/gamsuserconfig.h \
    option/lineeditcompleteevent.h \
    option/option.h \
    option/optioncompleterdelegate.h \
    option/optiondefinitionitem.h \
    option/optiondefinitionmodel.h \
    option/optionsortfilterproxymodel.h \
    option/optiontokenizer.h \
    option/paramconfigeditor.h \
    option/parametereditor.h \
    option/solveroptiondefinitionmodel.h \
    option/solveroptiontablemodel.h \
    option/solveroptionwidget.h \
    pinviewwidget.h \
    process.h \
    process/abstractprocess.h \
    process/connectprocess.h \
    process/gamsgetkeyprocess.h \
    process/gamslibprocess.h \
    process/gamsprocess.h \
    process/gmsunzipprocess.h \
    process/gmszipprocess.h \
    reference/filereferenceitem.h \
    reference/filereferencewidget.h \
    reference/fileusedtreemodel.h \
    reference/reference.h \
    reference/referencedatatype.h \
    reference/referenceitemmodel.h \
    reference/referencetabstyle.h \
    reference/referencetreemodel.h \
    reference/referenceviewer.h \
    reference/sortedfileheaderview.h \
    reference/symboldatatype.h \
    reference/symbolreferenceitem.h \
    reference/symbolreferencewidget.h \
    reference/symboltablemodel.h \
    search/abstractsearchfilehandler.h \
    search/fileworker.h \
    search/result.h \
    search/resultsview.h \
    search/search.h \
    search/searchdialog.h \
    search/searchfilehandler.h \
    search/searchhelpers.h \
    search/searchlocator.h \
    search/searchresultmodel.h \
    search/searchresultviewitemdelegate.h \
    search/searchworker.h \
    settings.h \
    settingsdialog.h \
    statuswidgets.h \
    support/checkforupdate.h \
    support/distributionvalidator.h \
    support/gamslicenseinfo.h       \
    support/gamslicensingdialog.h \
    support/solverconfiginfo.h      \
    support/updatewidget.h \
    support/versioninfoloader.h \
    svgengine.h \
    syntax.h \
    syntax/basehighlighter.h \
    syntax/blockcode.h \
    syntax/blockdata.h \
    syntax/htmlconverter.h \
    syntax/syntaxcommon.h \
    syntax/syntaxdeclaration.h \
    syntax/syntaxformats.h \
    syntax/syntaxhighlighter.h \
    syntax/syntaxidentifier.h \
    syntax/systemloghighlighter.h \
    syntax/textmark.h \
    syntax/textmarkrepo.h \
    tabbarstyle.h \
    tabdialog.h \
    tabwidget.h \
    theme.h \
    themewidget.h \
    version.h \
    viewhelper.h \
    welcomepage.h \
    wplabel.h

FORMS += \
    confirmdialog.ui \
    connect/connecteditor.ui \
    debugger/debugwidget.ui \
    encodingsdialog.ui \
    engine/efieditor.ui \
    engine/enginestartdialog.ui \
    file/pathrequest.ui \
    file/pathselect.ui \
    file/projectedit.ui \
    filesystemwidget.ui \
    gdxdiffdialog/gdxdiffdialog.ui \
    gdxviewer/columnfilterframe.ui \
    gdxviewer/exportdialog.ui \
    gdxviewer/gdxsymbolview.ui \
    gdxviewer/gdxviewer.ui \
    gdxviewer/valuefilterwidget.ui \
    lxiviewer/lxiviewer.ui \
    mainwindow.ui \
    miro/mirodeploydialog.ui \
    modeldialog/descriptiondialog.ui \
    modeldialog/modeldialog.ui \
    navigator/navigatordialog.ui \
    neos/neosstartdialog.ui \
    option/envvarconfigeditor.ui \
    option/gamsconfigeditor.ui \
    option/paramconfigeditor.ui \
    option/parametereditor.ui \
    option/solveroptionwidget.ui \
    pinviewwidget.ui \
    reference/filereferencewidget.ui \
    reference/referenceviewer.ui \
    reference/symbolreferencewidget.ui \
    search/resultsview.ui \
    search/searchdialog.ui \
    settingsdialog.ui \
    support/gamslicensingdialog.ui \
    support/updatewidget.ui \
    tabdialog.ui \
    themewidget.ui \
    welcomepage.ui

RESOURCES += \
    ../fonts/fonts.qrc \
    ../icons/icons.qrc

DISTFILES += \
    ../platform/windows/studio.rc

equals(QWEBENGINE, "true") {
DEFINES += QWEBENGINE
QT      += webenginewidgets
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
    ../ci/cloudfront-deploy.sh                      \
    ../ci/github-deploy.sh                          \
    ../ci/codechecker.sh                            \
    ../ci/qt-deploy.sh                              \
    ../ci/skipfile.txt                              \
    ../ci/.gitlab-ci-0-gams.yml                     \
    ../ci/.gitlab-ci-1-build.yml                    \
    ../ci/.gitlab-ci-2-test.yml                     \
    ../ci/.gitlab-ci-3-pack.yml                     \
    ../ci/.gitlab-ci-4-deploy.yml                   \
    ../ci/.gitlab-ci-5-analyze.yml                  \
    ../.gitlab-ci.yml                               \
    ../CHANGELOG                                    \
    ../README.md                                    \
    ../version
