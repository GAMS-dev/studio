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
#ifndef TESTOPTIONFILE_H
#define TESTOPTIONFILE_H

#include <QtTest/QTest>

#include "option/option.h"
#include "option/optiontokenizer.h"

using namespace gams::studio::option;

class TestOptionFile : public QObject
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

    void testOptionEnumStrType_data();
    void testOptionEnumStrType();

    void testOptionDoubleType_data();
    void testOptionDoubleType();

    void testOptionIntegerType_data();
    void testOptionIntegerType();

    void testOptionGroup_data();
    void testOptionGroup();

    void testOptionSynonym_data();
    void testOptionSynonym();

    void testDeprecatedOption_data();
    void testDeprecatedOption();

    void testHiddenOption_data();
    void testHiddenOption();

    void testInvalidOption_data();
    void testInvalidOption();

    void testReadOptionFile_data();
    void testReadOptionFile();

    void testNonExistReadOptionFile();

    void testWriteOptionFile_data();
    void testWriteOptionFile();

    void testIndicators_data();
    void testIndicators();

    void cleanupTestCase();

private:
    OptionTokenizer* optionTokenizer;
};

#endif // TESTOPTIONFILE_H
