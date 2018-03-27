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
#ifndef SYNTAXIDENTIFIER_H
#define SYNTAXIDENTIFIER_H

#include "syntaxformats.h"

namespace gams {
namespace studio {

class SyntaxIdentifier : public SyntaxAbstract
{
    QRegularExpression mRex;
public:
    SyntaxIdentifier(SyntaxState state);
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index) override;
};

class SyntaxIdentDescript : public SyntaxAbstract
{
    QChar mDelimiter;
    bool mTable;
public:
    SyntaxIdentDescript(SyntaxState state);
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index) override;
};

class SyntaxIdentAssign : public SyntaxAbstract
{
    QChar mDelimiter;
public:
    SyntaxIdentAssign(SyntaxState state);
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index) override;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXIDENTIFIER_H
