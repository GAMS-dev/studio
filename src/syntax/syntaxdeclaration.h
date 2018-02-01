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
