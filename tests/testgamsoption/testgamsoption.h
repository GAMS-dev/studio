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
#ifndef TESTGAMSOPTION_H
#define TESTGAMSOPTION_H

#include <QtTest/QTest>

#include "option/option.h"
#include "option/optiontokenizer.h"

using namespace gams::studio::option;

class TestGamsOption : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testOptionStringType_data();
    void testOptionStringType();

    void testOptionModelTypeString_data();
    void testOptionModelTypeString();

    void testOptionEnumStrType_data();
    void testOptionEnumStrType();

    void testOptionEnumStrValue_data();
    void testOptionEnumStrValue();

    void testOptionEnumIntType_data();
    void testOptionEnumIntType();

    void testOptionEnumIntValue_data();
    void testOptionEnumIntValue();

    void testOptionDoubleType_data();
    void testOptionDoubleType();

    void testOptionIntegerType_data();
    void testOptionIntegerType();

    void testOptionImmeidateType_data();
    void testOptionImmeidateType();

    void testOptionSynonym_data();
    void testOptionSynonym();

    void testDeprecatedOption_data();
    void testDeprecatedOption();

    void testDoubleDashedOption_data();
    void testDoubleDashedOption();

    void testOptionGroup_data();
    void testOptionGroup();

    void testInvalidOption_data();
    void testInvalidOption();

    void testTokenize_data();
    void testTokenize();

    void cleanupTestCase();

private:
    Option* gamsOption;
    OptionTokenizer *optionTokenizer;
};

#endif // TESTGAMSOPTION_H
