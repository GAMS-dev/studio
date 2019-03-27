#ifndef BASEHIGHLIGHTER_H
#define BASEHIGHLIGHTER_H

#include <QObject>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextObject>
#include <QVector>

namespace gams {
namespace studio {
namespace syntax {

class BaseHighlighter : public QObject
{
    Q_OBJECT
public:
    explicit BaseHighlighter(QObject *parent = nullptr);
    explicit BaseHighlighter(QTextDocument *parent = nullptr);
    virtual ~BaseHighlighter();

    void setDocument(QTextDocument *doc, bool wipe = false);
    QTextDocument *document() const;

public slots:
    void rehighlight();
    void rehighlightBlock(const QTextBlock &startBlock);

private slots:
    void reformatBlocks(int from, int charsRemoved, int charsAdded);
    void processDirtyParts();

protected:
    virtual void highlightBlock(const QString &text) = 0;
    void setFormat(int start, int count, const QTextCharFormat &format);
    QTextCharFormat format(int pos) const;

    int previousBlockState() const;
    int currentBlockState() const;
    void setCurrentBlockState(int newState);

    void setCurrentBlockUserData(QTextBlockUserData *data);
    QTextBlockUserData *currentBlockUserData() const;

    QTextBlock currentBlock() const;

private:
    void reformatCurrentBlock();
    void applyFormatChanges();
    void setDirty(int fromBlock, int toBlock = -1);
    void setClean(int fromBlock, int toBlock);
    inline int dirtyIndex(int blockNr) {
        for (int i = 0; i < mDirtyBlocks.size(); ++i) {
            if (mDirtyBlocks.at(i).first > blockNr || mDirtyBlocks.at(i).second > blockNr)
                return i;
        }
        return mDirtyBlocks.size();
    }
    inline QString debugDirty() {
        QString s;
        for (int i = 0; i < mDirtyBlocks.size() ; ++i) {
            s.append(QString::number(mDirtyBlocks.at(i).first)+"-"+QString::number(mDirtyBlocks.at(i).second)+" ");
        }
        return s;
    }

private:
    class Interval : public QPair<int,int>  {
    public:
        Interval(int first=0, int second=0) : QPair<int,int>(qMin(first, second), qMax(first, second)) {}
        bool isEmpty() { return first == second; }
        Interval subtractOverlap(const Interval &other);
        bool extendOverlap(const Interval &other);
        virtual ~Interval() {}
    };

    QTextDocument *mDoc = nullptr;
    QTextBlock mCurrentBlock;
    QVector<Interval> mDirtyBlocks;
    QVector<QTextCharFormat> mFormatChanges;
    static const int cMaxCount = 50;

};

} // namespace syntax
} // namespace studio
} // namespace gams

#endif // BASEHIGHLIGHTER_H
