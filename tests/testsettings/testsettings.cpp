/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "testsettings.h"
#include "settings.h"
#include "logger.h"

using namespace gams::studio;

void TestSettings::testWriteDefault()
{
    DEB() << "create:";
    Settings::createSettings(false, false, false);
    DEB() << "load:";
    Settings::settings()->load(Settings::KAll);
    DEB() << "save:";
    Settings::settings()->save();
    DEB() << "release:";
    Settings::settings()->releaseSettings();
}

QTEST_MAIN(TestSettings)
