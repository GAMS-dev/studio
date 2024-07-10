/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
namespace syntax {

class SyntaxIdentifier : public SyntaxAbstract
{
public:
    SyntaxIdentifier(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
private:
    int identChar(const QChar &c) const;
};

class SyntaxIdentifierDim : public SyntaxAbstract
{
    const QString mDelimiters;
public:
    SyntaxIdentifierDim(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
    int maxNesting() override { return 1; }
};

class SyntaxIdentifierDimEnd : public SyntaxAbstract
{
    const QString mDelimiters;
public:
    SyntaxIdentifierDimEnd(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxIdentDescript : public SyntaxAbstract
{
public:
    SyntaxIdentDescript(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxIdentAssign : public SyntaxAbstract
{
public:
    SyntaxIdentAssign(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class AssignmentLabel: public SyntaxAbstract
{
public:
    AssignmentLabel(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class AssignmentValue: public SyntaxAbstract
{
public:
    AssignmentValue(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxTableAssign : public SyntaxAbstract
{
public:
    SyntaxTableAssign(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxSimpleWord: public SyntaxAbstract
{
public:
    SyntaxSimpleWord(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};


} // namespace syntax
} // namespace studio
} // namespace gams

#endif // SYNTAXIDENTIFIER_H
