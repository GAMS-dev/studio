#ifndef TESTDIALOGFILEFILTER_H
#define TESTDIALOGFILEFILTER_H

#include <QtTest/QTest>
#include "editors/viewhelper.h"

using namespace gams::studio;

class TestDialogFileFilter : public QObject
{
    Q_OBJECT

private slots:
    void testUserCreatedTypes();
    void testAllFileTypes();
};

#endif // TESTDIALOGFILEFILTER_H
