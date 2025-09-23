TEMPLATE = app

include(../tests.pri)

INCLUDEPATH += $$SRCPATH/search

SOURCES +=  tst_testsearchcommon.cpp            \
            $$SRCPATH/search/searchcommon.cpp
