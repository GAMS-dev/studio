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
#ifndef SYNTAXDECLARATION_H
#define SYNTAXDECLARATION_H

#include "syntaxformats.h"

namespace gams {
namespace studio {
namespace syntax {

class DictEntry
{
public:
    DictEntry(QString entry): mEntry(entry)
    {
        for (int i = 0; i < entry.length(); ++i) {
            mInvertCase += entry.at(i).isLower() ? entry.at(i).toUpper() : entry.at(i).toLower();
        }
    }
    virtual ~DictEntry() {}
    inline bool is(const QChar &c, int i) const {
        return mEntry.at(i) == c || mInvertCase.at(i) == c;
    }
    inline int length() const { return mEntry.length(); }
private:
    QString mEntry;
    QString mInvertCase;
};

class DictList
{
public:
    DictList(QList<QPair<QString, QString>> list);
    virtual ~DictList() { while (!mEntries.isEmpty()) delete mEntries.takeFirst(); }
    inline int equalToPrevious(int i) const { return mEqualStart.at(i); }
    inline const DictEntry &at(int i) const { return *mEntries.at(i); }
    inline int count() const { return mEntries.length(); }
private:
    inline int equalStart(const QString &pre, const QString &post) const {
        int res = 0;
        while (res<pre.length() && res<post.length() && pre.at(res).toUpper() == post.at(res).toUpper()) res++;
        return res;
    }
    QVector<DictEntry*> mEntries;
    QVector<int> mEqualStart;
};


/// \brief Defines the syntax for a declaration.
class SyntaxKeywordBase: public SyntaxAbstract
{
public:
    ~SyntaxKeywordBase() override;
    SyntaxKeywordBase(SyntaxKind kind) : SyntaxAbstract(kind) {}
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;

protected:
    int findEnd(SyntaxKind kind, const QString& line, int index, int &entryIndex, bool openEnd = false);
    QHash<int, DictList*> mKeywords;

private:
    inline QStringList swapStringCase(QStringList list);
};

class SyntaxDeclaration: public SyntaxKeywordBase
{
public:
    SyntaxDeclaration();
    SyntaxBlock find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index) override;
};

class SyntaxPreDeclaration: public SyntaxKeywordBase
{
public:
    SyntaxPreDeclaration(SyntaxKind kind);
    SyntaxBlock find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index) override;
};


class SyntaxDeclarationTable: public SyntaxKeywordBase
{
public:
    SyntaxDeclarationTable();
    SyntaxBlock find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index) override;
};

class SyntaxReserved: public SyntaxKeywordBase
{
public:
    SyntaxReserved(SyntaxKind kind);
    SyntaxBlock find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index) override;
};

class SyntaxSubsetKey: public SyntaxKeywordBase
{
    QVector<int> mOtherKey;
public:
    SyntaxSubsetKey(SyntaxKind kind);
    SyntaxBlock find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index) override;
};

class SyntaxEmbedded: public SyntaxKeywordBase
{
public:
    SyntaxEmbedded(SyntaxKind kind);
    SyntaxBlock find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index) override;
};

class SyntaxEmbeddedBody: public SyntaxAbstract
{
public:
    SyntaxEmbeddedBody();
    SyntaxBlock find(const SyntaxKind entryKind, const int kindFlavor, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};

constexpr inline uint qHash(SyntaxKind key, uint seed = 0) noexcept { return uint(key) ^ seed; }

} // namespace syntax
} // namespace studio
} // namespace gans

#endif // SYNTAXDECLARATION_H
