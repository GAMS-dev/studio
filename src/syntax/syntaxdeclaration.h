/**
 * GAMS Studio
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
#ifndef SYNTAXDECLARATION_H
#define SYNTAXDECLARATION_H

#include "syntaxformats.h"

namespace gams {
namespace studio {
namespace syntax {

class DictEntry
{
public:
    DictEntry(const QString &entry): mEntry(entry)
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
    inline const QString &entry() const { return mEntry; }
private:
    QString mEntry;
    QString mInvertCase;
};

class DictList
{
public:
    DictList(QList<QPair<QString, QString>> list, const QString &prefix = QString());
    virtual ~DictList() { while (!mEntries.isEmpty()) delete mEntries.takeFirst(); }
    inline int equalToPrevious(int i) const { return mEqualStart.at(i); }
    inline const DictEntry &at(int i) const { return *mEntries.at(i); }
    inline const QString docAt(int i) const { return mDescript.at(i); }
    inline int count() const { return mEntries.length(); }
private:
    inline int equalStart(const QString &pre, const QString &post) const {
        int res = 0;
        while (res<pre.length() && res<post.length() && pre.at(res).toUpper() == post.at(res).toUpper()) res++;
        return res;
    }
    QVector<DictEntry*> mEntries;
    QVector<QString> mDescript;
    QVector<int> mEqualStart;
};

/// \brief Defines the syntax for a declaration.
class SyntaxKeywordBase: public SyntaxAbstract
{

public:
    ~SyntaxKeywordBase() override;
    SyntaxKeywordBase(SyntaxKind kind, SharedSyntaxData* sharedData)
        : SyntaxAbstract(kind, sharedData), mExtraKeywordChars("._") {}
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
    QStringList docForLastRequest() const override;

protected:
    int findEnd(SyntaxKind kind, const QString& line, int index, int &entryIndex, bool openEnd = false, bool relaxDot = false);
    void setExtraKeywordChars(const QString &keywordChars) { mExtraKeywordChars = keywordChars; }

    QHash<int, DictList*> mKeywords;
    QHash<int, int> mFlavors;
    QString mExtraKeywordChars;
    int mLastKind = -1;
    int mLastIKey = -1;

private:
    inline QStringList swapStringCase(const QStringList &list);
};

class SyntaxDeclaration: public SyntaxKeywordBase
{
public:
    SyntaxDeclaration(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
};

class SyntaxPreDeclaration: public SyntaxKeywordBase
{
public:
    SyntaxPreDeclaration(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
};


class SyntaxReserved: public SyntaxKeywordBase
{
public:
    SyntaxReserved(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
};

class SyntaxSimpleKeyword: public SyntaxKeywordBase
{
public:
    SyntaxSimpleKeyword(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxSubsetKey: public SyntaxKeywordBase
{
    QVector<int> mOtherKey;
public:
    SyntaxSubsetKey(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class SyntaxEmbedded: public SyntaxKeywordBase
{
public:
    SyntaxEmbedded(SyntaxKind kind, SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
};

class SyntaxEmbeddedBody: public SyntaxAbstract
{
public:
    SyntaxEmbeddedBody(SharedSyntaxData* sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

class AssignmentSystemData: public SyntaxKeywordBase
{
public:
    AssignmentSystemData(SharedSyntaxData *sharedData);
    SyntaxBlock find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index) override;
    SyntaxBlock validTail(const QString &line, int index, SyntaxState state, bool &hasContent) override;
};

constexpr inline size_t qHash(SyntaxKind key, size_t seed = 0) noexcept { return size_t(key) ^ seed; }

} // namespace syntax
} // namespace studio
} // namespace gans

#endif // SYNTAXDECLARATION_H
