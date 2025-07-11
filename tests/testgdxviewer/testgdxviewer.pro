TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH

SOURCES += \
    tst_testgdxviewer.cpp \
    $$SRCPATH/abstractview.cpp \
    $$SRCPATH/commonpaths.cpp \
    $$SRCPATH/common.cpp                        \
    $$SRCPATH/editors/abstracttextmapper.cpp \
    $$SRCPATH/editors/chunktextmapper.cpp \
    $$SRCPATH/editors/defaultsystemlogger.cpp \
    $$SRCPATH/editors/editorhelper.cpp \
    $$SRCPATH/editors/logparser.cpp \
    $$SRCPATH/editors/sysloglocator.cpp \
    $$SRCPATH/exception.cpp \
    $$SRCPATH/filterlineedit.cpp \
    $$SRCPATH/gdxviewer/columnfilter.cpp \
    $$SRCPATH/gdxviewer/columnfilterframe.cpp \
    $$SRCPATH/gdxviewer/exportdialog.cpp \
    $$SRCPATH/gdxviewer/exportdriver.cpp \
    $$SRCPATH/gdxviewer/exportmodel.cpp \
    $$SRCPATH/gdxviewer/filteruelmodel.cpp \
    $$SRCPATH/gdxviewer/gdxsymbol.cpp \
    $$SRCPATH/gdxviewer/gdxsymbolheaderview.cpp \
    $$SRCPATH/gdxviewer/gdxsymboltablemodel.cpp \
    $$SRCPATH/gdxviewer/gdxsymboltableview.cpp \
    $$SRCPATH/gdxviewer/gdxsymbolview.cpp \
    $$SRCPATH/gdxviewer/gdxsymbolviewstate.cpp \
    $$SRCPATH/gdxviewer/gdxviewer.cpp \
    $$SRCPATH/gdxviewer/gdxviewerstate.cpp \
    $$SRCPATH/gdxviewer/nestedheaderview.cpp \
    $$SRCPATH/gdxviewer/numericalformatcontroller.cpp \
    $$SRCPATH/gdxviewer/quickselectlistview.cpp \
    $$SRCPATH/gdxviewer/tabenabledmenu.cpp \
    $$SRCPATH/gdxviewer/tableviewdomainmodel.cpp \
    $$SRCPATH/gdxviewer/tableviewmodel.cpp \
    $$SRCPATH/gdxviewer/valuefilter.cpp \
    $$SRCPATH/gdxviewer/valuefilterwidget.cpp \
    $$SRCPATH/headerviewproxy.cpp \
    $$SRCPATH/logger.cpp \
    $$SRCPATH/numerics/doubleFormat.c \
    $$SRCPATH/numerics/doubleformatter.cpp \
    $$SRCPATH/../extern/dtoaloc/dtoaLoc.c \
    $$SRCPATH/process/abstractprocess.cpp \
    $$SRCPATH/process/connectprocess.cpp \
    $$SRCPATH/settings.cpp \
    $$SRCPATH/svgengine.cpp \
    $$SRCPATH/encoding.cpp \
    $$SRCPATH/theme.cpp


HEADERS += \
    $$SRCPATH/abstractview.h \
    $$SRCPATH/common.h \
    $$SRCPATH/commonpaths.h \
    $$SRCPATH/editors/abstractsystemlogger.h \
    $$SRCPATH/editors/abstracttextmapper.h \
    $$SRCPATH/editors/chunktextmapper.h \
    $$SRCPATH/editors/defaultsystemlogger.h \
    $$SRCPATH/editors/editorhelper.h \
    $$SRCPATH/editors/logparser.h \
    $$SRCPATH/editors/sysloglocator.h \
    $$SRCPATH/exception.h \
    $$SRCPATH/filterlineedit.h \
    $$SRCPATH/gdxviewer/columnfilter.h \
    $$SRCPATH/gdxviewer/columnfilterframe.h \
    $$SRCPATH/gdxviewer/exportdialog.h \
    $$SRCPATH/gdxviewer/exportdriver.h \
    $$SRCPATH/gdxviewer/exportmodel.h \
    $$SRCPATH/gdxviewer/filteruelmodel.h \
    $$SRCPATH/gdxviewer/gdxsymbol.h \
    $$SRCPATH/gdxviewer/gdxsymbolheaderview.h \
    $$SRCPATH/gdxviewer/gdxsymboltablemodel.h \
    $$SRCPATH/gdxviewer/gdxsymboltableview.h \
    $$SRCPATH/gdxviewer/gdxsymbolview.h \
    $$SRCPATH/gdxviewer/gdxsymbolviewstate.h \
    $$SRCPATH/gdxviewer/gdxviewer.h \
    $$SRCPATH/gdxviewer/gdxviewerstate.h \
    $$SRCPATH/gdxviewer/numericalformatcontroller.cpp \
    $$SRCPATH/gdxviewer/nestedheaderview.h \
    $$SRCPATH/gdxviewer/quickselectlistview.h \
    $$SRCPATH/gdxviewer/tabenabledmenu.h \
    $$SRCPATH/gdxviewer/tableviewdomainmodel.h \
    $$SRCPATH/gdxviewer/tableviewmodel.h \
    $$SRCPATH/gdxviewer/valuefilter.h \
    $$SRCPATH/gdxviewer/valuefilterwidget.h \
    $$SRCPATH/headerviewproxy.h \
    $$SRCPATH/logger.h \
    $$SRCPATH/numerics/doubleFormat.h \
    $$SRCPATH/numerics/doubleformatter.h \
    $$SRCPATH/../extern/dtoaloc/dtoaLoc.h \
    $$SRCPATH/process.h \
    $$SRCPATH/process/abstractprocess.h \
    $$SRCPATH/process/connectprocess.h \
    $$SRCPATH/settings.h \
    $$SRCPATH/svgengine.h \
    $$SRCPATH/encoding.h \
    $$SRCPATH/theme.h
