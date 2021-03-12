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
#include "testgotodialog.h"
#include "gotodialog.h"

#include <QLineEdit>
#include <QPushButton>

void TestGoToDialog::initTestCase()
{
    mDialog = new GoToDialog(nullptr, 100);
}

void TestGoToDialog::cleanupTestCase()
{
    delete mDialog;
}

void TestGoToDialog::testit()
{
    // One file changed externally
    QLineEdit* ed = mDialog->findChild<QLineEdit*>("lineEdit");
    ed->setText("50");
    QTest::mouseClick(mDialog->findChild<QPushButton*>("goToButton"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == 1, "GoToDialog", "line number in range");

    ed->setText("150");
    QTest::mouseClick(mDialog->findChild<QPushButton*>("goToButton"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == 0, "GoToDialog", "line number out of range");
}

QTEST_MAIN(TestGoToDialog)
