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
#ifndef TESTGUROBIOPTION_H
#define TESTGUROBIOPTION_H

#include <QtTest/QTest>

#include "option/option.h"
#include "option/optiontokenizer.h"

using namespace gams::studio::option;

class TestGUROBIOption : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testOptionBooleanType_data();
    void testOptionBooleanType();

    void testOptionStringType_data();
    void testOptionStringType();

    void testOptionEnumIntType_data();
    void testOptionEnumIntType();

    void testOptionDoubleType_data();
    void testOptionDoubleType();

    void testOptionIntegerType_data();
    void testOptionIntegerType();

    void testDeprecatedOption_data();
    void testDeprecatedOption();

    void testOptionGroup_data();
    void testOptionGroup();

    void testReadOptionFile_data();
    void testReadOptionFile();

    void testNonExistReadOptionFile();

    void testWriteOptionFile_data();
    void testWriteOptionFile();

    void testEOLChars();

    void cleanupTestCase();

private:
    bool containKey(QList<OptionItem> &items, const QString &key) const;
    QVariant getValue(QList<OptionItem> &items, const QString &key) const;

    bool Dcreated = false;
    bool optdefRead = false;
    optHandle_t mOPTHandle;

    OptionTokenizer* optionTokenizer;
};

#endif // TESTGUROBIOPTION_H
