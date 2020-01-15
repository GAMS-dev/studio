#include "testdialogfilefilter.h"

void TestDialogFileFilter::testUserCreatedTypes()
{
    QStringList userFileTypes = {"*.gms", "*.txt", "*.opt", "*.op*", "*.o*", "*.*"};

    for (QString t : userFileTypes) {
        QVERIFY2(ViewHelper::dialogFileFilterUserCreated().filter(t).size() > 0,
                 QString("%1 not part of Dialog File Filter for user created files.").arg(t).toStdString().c_str());
    }
}

void TestDialogFileFilter::testAllFileTypes()
{
    QStringList gamsFileTypes = {".gms", ".txt", ".opt", ".op*", ".o*", "*.*", "*.gdx", "*.log", "*.lst", "*.ref", "*.dmp"};
    for (QString t : gamsFileTypes) {
        QVERIFY2(ViewHelper::dialogFileFilterAll().filter(t).size() > 0,
                 QString("%1 not part of Dialog File Filter for GAMS created files.").arg(t).toStdString().c_str());
    }
}

QTEST_MAIN(TestDialogFileFilter)
