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
#ifndef TESTCOMPLETER_H
#define TESTCOMPLETER_H

#include <QTest>
#include "syntaxsimulator.h"

namespace gams {
namespace studio {
class CodeCompleter;
}
}

class TestCompleter : public QObject
{
    Q_OBJECT

private:
    gams::studio::CodeCompleter *mCompleter = nullptr;
    SyntaxSimulator mSynSim;

    QByteArray describe(int receive, int expect, QStringList types);

private slots:
    void initTestCase();

    void testDco();
    void testDeclaration();
    void testPut();

};


#endif // TESTCOMPLETER_H
