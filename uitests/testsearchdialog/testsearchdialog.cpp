/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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
#include "testsearchdialog.h"

#include <QLineEdit>
#include <QPushButton>

using namespace search;

void TestSearchDialog::initTestCase()
{
    TestFileHandler* tfh;
    mDialog = new SearchDialog(tfh);
}

void TestSearchDialog::cleanupTestCase()
{
    delete mDialog;
}

void TestSearchDialog::test_availability()
{
    mDialog->show();
    QTEST_ASSERT(mDialog->findChild<QPushButton*>("btn_Replace"));
    QTEST_ASSERT(mDialog->findChild<QPushButton*>("btn_clear"));
    QTEST_ASSERT(mDialog->findChild<QComboBox*>("combo_scope"));
    QTEST_ASSERT(mDialog->findChild<QComboBox*>("combo_filepattern"));
    QTEST_ASSERT(mDialog->findChild<QLabel*>("lbl_nrResults"));
}

QTEST_MAIN(TestSearchDialog)
