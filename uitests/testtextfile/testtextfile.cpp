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
#include "testtextfile.h"
#include "studiosettings.h"
#include "locators/settingslocator.h"
#include "editors/viewhelper.h"
#include "commonpaths.h"

#include <QClipboard>


void testTextFile::initTestCase()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);

    mSettings = new StudioSettings(true, false, false);
    SettingsLocator::provide(mSettings);

    mMainWindow = new MainWindow();
    QVERIFY(mMainWindow);
    mMainWindow->show();
}

void testTextFile::init()
{
    mEmptyFile = QFileInfo(mSettings->defaultWorkspace()+"/empty.txt");
    QFile f(mEmptyFile.filePath());

    // create empty file
    if (mEmptyFile.exists())
        Q_ASSERT(f.remove());
    QTest::qWait(100);
    Q_ASSERT(f.open(QIODevice::WriteOnly));
    f.close();

    mEmptyFile.refresh();
    Q_ASSERT(mEmptyFile.exists());

    mMainWindow->openFilePath(mEmptyFile.filePath(), true);
    QTest::qWait(100);

    // check if right file focussed
    Q_ASSERT(mMainWindow->recent()->editor());
    QVERIFY2(mMainWindow->recent()->editor()->property("location").toString() == mEmptyFile.filePath(), "Wrong file focussed. Expected: empty.txt");
}

void testTextFile::cleanup()
{
    CodeEdit* ce = ViewHelper::toCodeEdit(mMainWindow->recent()->editor());
    ce->document()->setPlainText("");
}

void testTextFile::testWriting()
{
    CodeEdit* ce = ViewHelper::toCodeEdit(mMainWindow->recent()->editor());
    QVERIFY2(ce->document()->toRawText().isEmpty(), "Document not empty");
    QString testString("this is my test");
    QTest::keyClicks(ce, testString);
    QVERIFY2(ce->document()->toRawText().size() == testString.size(), "Different size of input and document content");
    QVERIFY2(ce->document()->toRawText() == testString, "Document content not identical to input");
}

void testTextFile::testClipboard()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    QString testString("this is my test");
    clipboard->setText(testString);

    CodeEdit* ce = ViewHelper::toCodeEdit(mMainWindow->recent()->editor());
    QVERIFY2(ce->document()->toRawText().isEmpty(), "Document not empty");

}

QTEST_MAIN(testTextFile)
