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
#ifndef TEST_MAIN_WINDOW_H
#define TEST_MAIN_WINDOW_H

#include <QtTest/QTest>
#include "mainwindow.h"

using namespace gams::studio;

class TestMainWindow : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
//    void cleanup();
    void cleanupTestCase();

    void test_gdxValue();
//    void test_search();

private:
    MainWindow* mMainWindow = nullptr;
    Settings* mSettings = nullptr;
    QFileInfo mGms;
    void clickRowByName(QTableView* source, const QString& name);
};

#endif // TEST_MAIN_WINDOW_H
