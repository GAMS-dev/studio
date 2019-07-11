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

#include <QDialog>
#include <QToolBar>

void testmainwindow::initTestCase()
{
    mSettings = new StudioSettings(true, false, false);
    SettingsLocator::provide(mSettings);

    mMainWindow = new MainWindow();
    QVERIFY(mMainWindow);

    // make sure a file to work with is there
    mGms = QFileInfo(mSettings->defaultWorkspace()+"/trnsport.gms");
    if (!mGms.exists())
        mMainWindow->receiveModLibLoad("trnsport");
    Q_ASSERT(mGms.exists());
}

void testmainwindow::cleanupTestCase()
{
    delete mMainWindow;
    delete mSettings;
}


QTEST_MAIN(testmainwindow)
