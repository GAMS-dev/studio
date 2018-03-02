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
#include <QtCore>

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
    DictList(QStringList list) {
        list.sort(Qt::CaseInsensitive);
        QString last;
        for (QString s: list)  {
            mEntries << new DictEntry(s);
            mEqualStart << equalStart(last,s);
            last = s;
        }
    }
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
class SyntaxDeclaration: public SyntaxAbstract
{
public:
    SyntaxDeclaration(SyntaxState state);
    ~SyntaxDeclaration();
    inline SyntaxState state() override { return mState; }
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;

private:
    inline QStringList swapStringCase(QStringList list);
//    inline bool isWhitechar(const QString& line, int index);
    int findEnd(SyntaxState state, const QString& line, int index);

private:
    const SyntaxState mState;
    QRegularExpression mRex;
    QHash<SyntaxState, DictList*> mKeywords;

};

constexpr inline uint qHash(SyntaxState key, uint seed = 0) noexcept { return uint(key) ^ seed; }

} // namespace studio
} // namespace gans

#endif // SYNTAXDECLARATION_H
