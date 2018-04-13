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
#ifndef SYNTAXDECLARATION_H
#define SYNTAXDECLARATION_H

#include "syntaxformats.h"

namespace gams {
namespace studio {

class DictEntry
{
public:
    DictEntry(QString entry): mEntry(entry), mUpCase(entry.toUpper()), mLoCase(entry.toLower()) {}
    virtual ~DictEntry() {}
    inline bool is(const QChar &c, int i) const {
        return c.isUpper() ? mUpCase.at(i) == c : mLoCase.at(i) == c;
    }
    inline bool after(const QChar &c, int i) const {
        return c.isUpper() ? mUpCase.at(i) > c  : mLoCase.at(i) > c;
    }
    inline int length() const { return mEntry.length(); }
private:
    QString mEntry;
    QString mUpCase;
    QString mLoCase;
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
    ~SyntaxKeywordBase();
    SyntaxKeywordBase(SyntaxState state) : SyntaxAbstract(state) {}
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;

protected:
    int findEnd(SyntaxState state, const QString& line, int index);
    QHash<SyntaxState, DictList*> mKeywords;

private:
    inline QStringList swapStringCase(QStringList list);
};

class SyntaxDeclaration: public SyntaxKeywordBase
{
public:
    SyntaxDeclaration();
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index);
};

class SyntaxPreDeclaration: public SyntaxKeywordBase
{
public:
    SyntaxPreDeclaration(SyntaxState state);
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index);
};


class SyntaxDeclarationTable: public SyntaxKeywordBase
{
public:
    SyntaxDeclarationTable();
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
};

class SyntaxReserved: public SyntaxKeywordBase
{
public:
    SyntaxReserved();
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
};

class SyntaxReservedBody: public SyntaxAbstract
{
public:
    SyntaxReservedBody();
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, bool &hasContent) override;
};

constexpr inline uint qHash(SyntaxState key, uint seed = 0) noexcept { return uint(key) ^ seed; }

} // namespace studio
} // namespace gans

#endif // SYNTAXDECLARATION_H
