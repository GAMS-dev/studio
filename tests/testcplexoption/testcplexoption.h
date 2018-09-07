/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef TESTCPLEXOPTION_H
#define TESTCPLEXOPTION_H

#include <QtTest/QTest>

#include "option/option.h"

using gams::studio::Option;

class TestCPLEXOption : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testOptionEnumStrValue_data();
    void testOptionEnumStrValue();

    void testOptionEnumIntType_data();
    void testOptionEnumIntType();

    void testOptionEnumIntValue_data();
    void testOptionEnumIntValue();

    void testOptionDoubleType_data();
    void testOptionDoubleType();

    void testOptionSynonym_data();
    void testOptionSynonym();

    void testDeprecatedOption_data();
    void testDeprecatedOption();

    void testOptionGroup_data();
    void testOptionGroup();

    void testInvalidOption_data();
    void testInvalidOption();

    void testReadOptionFile();
    void testWriteOptionFile();

    void cleanupTestCase();

private:
    Option* cplexOption;
};

#endif // TESTCPLEXOPTION_H
