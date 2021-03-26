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
#include "testfilechangedialog.h"
#include "file/filechangedialog.h"

void TestFileChangeDialog::initTestCase()
{
    mDialog = new FileChangeDialog();
}

void TestFileChangeDialog::cleanupTestCase()
{
    delete mDialog;
}

void TestFileChangeDialog::test_single()
{
    // One file changed externally
    mDialog->show("C:\\test\\aFile.txt", false, false, 1);
    QTEST_ASSERT_X(!mDialog->findChild<QCheckBox*>("cbAll")->isVisible(), "FileChangeDialog", "Checkbox visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QCheckBox*>("cbAll")->isChecked(), "FileChangeDialog", "Checkbox checked");
    QTEST_ASSERT_X(!mDialog->findChild<QPushButton*>("btClose")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btReload")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btReloadAlways")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btKeep")->isVisible(), "FileChangeDialog", "Button visiblity");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btReload"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rReload), "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btReloadAlways"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rReloadAlways), "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btKeep"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rKeep), "FileChangeDialog", "Result error");

    // One file deleted externally
    mDialog->show("C:\\test\\aFile.txt", true, false, 1);
    QTEST_ASSERT_X(!mDialog->findChild<QCheckBox*>("cbAll")->isVisible(), "FileChangeDialog", "Checkbox visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QCheckBox*>("cbAll")->isChecked(), "FileChangeDialog", "Checkbox checked");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btClose")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QPushButton*>("btReload")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QPushButton*>("btReloadAlways")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btKeep")->isVisible(), "FileChangeDialog", "Button visiblity");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btClose"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rClose), "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btKeep"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rKeep), "FileChangeDialog", "Result error");
}

void TestFileChangeDialog::test_multi()
{
    int allFlag = int(FileChangeDialog::Result::rCount);
    // Three files changed externally
    mDialog->show("C:\\test\\aFile.txt", false, false, 3);
    QTEST_ASSERT_X(mDialog->findChild<QCheckBox*>("cbAll")->isVisible(), "FileChangeDialog", "Checkbox visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QCheckBox*>("cbAll")->isChecked(), "FileChangeDialog", "Checkbox checked");
    QTEST_ASSERT_X(!mDialog->findChild<QPushButton*>("btClose")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btReload")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btReloadAlways")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btKeep")->isVisible(), "FileChangeDialog", "Button visiblity");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btReload"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rReload), "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btReloadAlways"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rReloadAlways), "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btKeep"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rKeep), "FileChangeDialog", "Result error");

    // check cbAll
    QTest::mouseClick(mDialog->findChild<QCheckBox*>("cbAll"), Qt::LeftButton);

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btReload"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rReload)+allFlag, "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btReloadAlways"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rReloadAlways)+allFlag, "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btKeep"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rKeep)+allFlag, "FileChangeDialog", "Result error");


    // Three files deleted externally
    mDialog->show("C:\\test\\aFile.txt", true, false, 3);
    QTEST_ASSERT_X(mDialog->findChild<QCheckBox*>("cbAll")->isVisible(), "FileChangeDialog", "Checkbox visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QCheckBox*>("cbAll")->isChecked(), "FileChangeDialog", "Checkbox checked");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btClose")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QPushButton*>("btReload")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(!mDialog->findChild<QPushButton*>("btReloadAlways")->isVisible(), "FileChangeDialog", "Button visiblity");
    QTEST_ASSERT_X(mDialog->findChild<QPushButton*>("btKeep")->isVisible(), "FileChangeDialog", "Button visiblity");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btClose"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rClose), "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btKeep"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rKeep), "FileChangeDialog", "Result error");

    // check cbAll
    QTest::mouseClick(mDialog->findChild<QCheckBox*>("cbAll"), Qt::LeftButton);

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btClose"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rClose)+allFlag, "FileChangeDialog", "Result error");

    QTest::mouseClick(mDialog->findChild<QPushButton*>("btKeep"), Qt::LeftButton);
    QTEST_ASSERT_X(mDialog->result() == int(FileChangeDialog::Result::rKeep)+allFlag, "FileChangeDialog", "Result error");
}

QTEST_MAIN(TestFileChangeDialog)
