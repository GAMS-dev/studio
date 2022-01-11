/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "testmodeldialog.h"
#include "commonpaths.h"
#include "mainwindow.h"

#include <QTabWidget>
#include <QLineEdit>
#include <QTableView>

void testmodeldialog::initTestCase()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);

    mDialog = new ModelDialog();
}

void testmodeldialog::cleanupTestCase()
{
    delete mDialog;
}

void testmodeldialog::test_tabWidget()
{
    // test search
    QTabWidget* tabs = mDialog->findChild<QTabWidget*>("tabWidget");
    QVERIFY2(tabs, "Model Library tab widget not found.");

    QVERIFY2(tabs->count() >= 8, "Not all default libraries loaded.");

    QLineEdit* search = mDialog->findChild<QLineEdit*>("lineEdit");
    QVERIFY2(search, "Model Library search bar not found.");

    search->setText("trnsport");
    QTableView* tv = mDialog->tableAt(tabs->currentIndex());
    QVERIFY2(tv, "Model Library Table not found.");

    QCOMPARE(tv->model()->rowCount(), 1);

}

void testmodeldialog::test_fileLoad()
{
    QTabWidget* tabs = mDialog->findChild<QTabWidget*>("tabWidget");
    QVERIFY2(tabs, "Model Library tab widget not found.");

    QTableView* tv = mDialog->tableAt(tabs->currentIndex());
    QVERIFY2(tv, "Model Library Table not found.");

//    following part needs mainwindow:
//    tv->selectRow(0);
//    LibraryItem *item = mDialog->selectedLibraryItem();
//    mMainWindow.triggerGamsLibFileCreation(item);
}

QTEST_MAIN(testmodeldialog)
