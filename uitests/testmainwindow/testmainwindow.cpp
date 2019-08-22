/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#include "testmainwindow.h"
#include "studiosettings.h"
#include "locators/settingslocator.h"
#include "modeldialog/modeldialog.h"
#include "search/searchdialog.h"
#include "commonpaths.h"

#include <QDialog>
#include <QToolBar>

void testmainwindow::initTestCase()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);

    mSettings = new StudioSettings(true, false, false);
    SettingsLocator::provide(mSettings);

    mMainWindow = new MainWindow();
    QVERIFY(mMainWindow);

    // make sure a file to work with is there
    mMainWindow->receiveModLibLoad("trnsport", true);
    QTest::qWait(200); // needed for filesystem to finish writing

    mGms = QFileInfo(mSettings->defaultWorkspace()+"/trnsport.gms");
    Q_ASSERT(mGms.exists());
}

void testmainwindow::cleanupTestCase()
{
    delete mMainWindow;
    delete mSettings;
}

void testmainwindow::test_search()
{
    mMainWindow->openFilePath(mGms.filePath(), true);
    Q_ASSERT(mMainWindow->recent()->editor());
    QVERIFY2(mMainWindow->recent()->editor()->property("location").toString() == mGms.filePath(), "Wrong file focussed.");

    SearchDialog* sd = mMainWindow->findChild<SearchDialog*>("SearchDialog");

    QVERIFY2(!sd->isVisible(), "Search Dialog already visible. Something is wrong.");
    mMainWindow->openSearchDialog();
    QVERIFY2(sd->isVisible(), "Search Dialog not visible. Something is wrong.");

    QComboBox* input = mMainWindow->findChild<QComboBox*>("combo_search");
    QVERIFY2(input, "Input field not found");

    QVERIFY2(!sd->resultsView(), "ResultsView still open. Something is wrong.");
    QString searchword("text");
    QTest::keyClicks(input, searchword);

    QWidget* btnFindAll = sd->findChild<QWidget*>("btn_FindAll");
    QVERIFY2(btnFindAll, "Find All Button not found");
    QTest::mouseClick(btnFindAll, Qt::LeftButton);

    QTest::qWait(500); // wait for asynchronous search

    QVERIFY2(input->currentText() == searchword, "Text in search field differs from input text.");
    QVERIFY2(sd->resultsView(), "ResultsView did not open");

    QVERIFY2(sd->results(), "SearchResultList not available.");
    QCOMPARE(sd->results()->size(), 2);
}

QTEST_MAIN(testmainwindow)
