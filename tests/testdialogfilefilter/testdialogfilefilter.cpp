/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "testdialogfilefilter.h"

void TestDialogFileFilter::testUserCreatedTypes()
{
    QStringList userFileTypes = {"*.gms", "*.txt", "*.opt", "*.op*", "*.o*", allFilesFilter};

    for (const QString &t : userFileTypes) {
        QVERIFY2(ViewHelper::dialogFileFilterUserCreated().filter(t).size() > 0,
                 QString("%1 not part of Dialog File Filter for user created files.").arg(t).toStdString().c_str());
    }
}

void TestDialogFileFilter::testAllFileTypes()
{
    QStringList gamsFileTypes = {".gms", ".txt", ".opt", ".op*", ".o*", allFilesFilter, "*.gdx", "*.log", "*.lst", "*.ref", "*.dmp"};
    for (const QString &t : gamsFileTypes) {
        QVERIFY2(ViewHelper::dialogFileFilterAll().filter(t).size() > 0,
                 QString("%1 not part of Dialog File Filter for GAMS created files.").arg(t).toStdString().c_str());
    }
}

QTEST_MAIN(TestDialogFileFilter)
