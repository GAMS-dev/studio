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
#include "settingslocator.h"
#include "modeldialog/modeldialog.h"
#include "search/searchdialog.h"
#include "commonpaths.h"
#include "gdxviewer/gdxsymbolview.h"

#include <QDialog>
#include <QSplitter>
#include <QToolBar>

using namespace gdxviewer;
using namespace option;

void testmainwindow::initTestCase()
{
    CommonPaths::setSystemDir(GAMS_DISTRIB_PATH);

    mSettings = new StudioSettings(true, false, false);
    SettingsLocator::provide(mSettings);

    mMainWindow = new MainWindow();
    QVERIFY(mMainWindow);
    mMainWindow->show();
}

void testmainwindow::init()
{
    // make sure a file to work with is there
    mMainWindow->receiveModLibLoad("trnsport", true);
    QTest::qWait(100); // needed for filesystem to finish writing

    mGms = QFileInfo(mSettings->defaultWorkspace()+"/trnsport.gms");
    Q_ASSERT(mGms.exists());
}

void testmainwindow::cleanupTestCase()
{
//    delete mMainWindow; // crashes at end of test
    delete mSettings;
}

void testmainwindow::clickRowByName(QTableView* source, const QString& name)
{
    QAbstractItemModel *model = source->model();
    QModelIndexList matches = model->match(model->index(0,1), Qt::DisplayRole, name, 1, Qt::MatchFixedString);

    // click on x
    QModelIndex symX = matches.first();
    int xPos = source->columnViewportPosition(symX.column()) + 5;
    int yPos = source->rowViewportPosition(symX.row()) + 5;
    QWidget* pViewport = source->viewport();
    QTest::mouseClick(pViewport, Qt::LeftButton, NULL, QPoint(xPos, yPos));
    QTest::qWait(100);
}

void testmainwindow::test_gdxValue()
{
//    - Press F6
//    - Select trnsport
//    - Add rf=xxx to the command line
//    - Press F10
//    - Open trnsport.gdx and check that the result for x is right
//    - Check that table view of x works
//    - Open xxx.rf
//    - Look at the entries of d

    // go to trnsport.gms
    mMainWindow->openFilePath(mGms.filePath(), true);
    Q_ASSERT(mMainWindow->recent()->editor());
    QVERIFY2(mMainWindow->recent()->editor()->property("location").toString() == mGms.filePath(), "Wrong file focussed. Expected: trnsport.gms");

    // set gams options
    CommandLineOption* combobox = mMainWindow->gamsOptionWidget()->findChild<option::CommandLineOption*>("gamsOptionCommandLine");
    Q_ASSERT(combobox);
    QString refFileName = "xxx.rf";
    combobox->setCurrentText("rf=" + refFileName);

    // run gams
    QTest::keyEvent(QTest::Click, mMainWindow, Qt::Key_F10, Qt::NoModifier);
    QTest::qWait(500); // wait for gams to run
    QApplication::processEvents();

    // open gdx
    QFileInfo gdx(mGms.path() + "/" + mGms.baseName() + ".gdx");
    mMainWindow->openFilePath(gdx.filePath());
    QVERIFY2(mMainWindow->recent()->editor()->property("location").toString() == gdx.filePath(), "Wrong file focussed. Expected: trnsport.gdx");
    GdxViewer* gdxViewer = static_cast<GdxViewer*>(mMainWindow->recent()->editor());
    Q_ASSERT(gdxViewer);

    // find x
    QTableView* tvSymbols = gdxViewer->findChild<QTableView*>("tvSymbols");
    Q_ASSERT(tvSymbols);
    clickRowByName(tvSymbols, "x");

    // checking value of x
    QSplitter* splitter = gdxViewer->findChild<QSplitter*>("splitter");
    GdxSymbolView* symbolView = static_cast<GdxSymbolView*>(splitter->widget(1)); // this is shakey
    // TODO(rg): get qtableview from gdxsymbolview, or find some other way

    // open ref file
    QFileInfo ref(mGms.path() + "/" + refFileName);
    mMainWindow->openFilePath(ref.filePath());
    QTest::qWait(500);
    QApplication::processEvents();
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
