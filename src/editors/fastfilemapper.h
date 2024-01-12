#ifndef GAMS_STUDIO_FASTFILEMAPPER_H
#define GAMS_STUDIO_FASTFILEMAPPER_H

#include "abstracttextmapper.h"
#include <QFile>
#include <QMutex>
#include <QQueue>

namespace gams {
namespace studio {

struct CacheElement
{
    CacheElement(int _line, int _count): line(_line), count(_count) {}
    CacheElement(int _line, int _count, const QString &_lines): line(_line), count(_count), lines(_lines) {}
    int line;
    int count;
    QString lines;
    bool operator==(const CacheElement &other) const { return line == other.line && count == other.count; }
    void operator =(const CacheElement &other) { line = other.line; count = other.count; lines = other.lines; }
};

class FastFileMapper : public AbstractTextMapper
{
private:
    friend class LinesCache;
    class LinesCache
    {
        FastFileMapper *mMapper;
        mutable QString mData;
        mutable int mLastLineLength = -1;
        mutable QList<qint64> mLineChar;    // start of lines in mCache (these are characters, not bytes!)
        mutable int mCacheOffsetLine;
        mutable QQueue<CacheElement> mDirectCache;
    public:
        LinesCache(FastFileMapper *mapper) : mMapper(mapper) { reset(); }
        virtual ~LinesCache() {}
        void reset() const;
        const QString loadCache(int lineNr, int count) const;
        QString getLines(int lineNr, int count) const;
        int cachedLineCount() const;
        QPoint posForOffset(int offset);
        int lineLength(int lineNr) const;
        int firstCacheLine() const { return mCacheOffsetLine; }
        int lastCacheLine() const { return mCacheOffsetLine + mLineChar.size() - 1; }
        qint64 linePos(int line) const;
    };

    Q_OBJECT
public:
    enum Field { fVirtualLastLineEnd, fCacheFirst, fCacheLast, fPosLineStartInFile };

    explicit FastFileMapper(QObject *parent = nullptr);
    ~FastFileMapper() override;
    virtual AbstractTextMapper::Kind kind() const override;

    bool openFile(const QString &fileName, bool initAnchor);
    QString fileName() const;
    qint64 size() const override;

    void startRun() override;
    void endRun() override;

    bool setVisibleTopLine(double region) override;
    bool setVisibleTopLine(int lineNr) override;
    int moveVisibleTopLine(int lineDelta) override;
    int visibleTopLine() const override;
    void scrollToPosition() override;

    int lineCount() const override;
    int knownLineNrs() const override;

    QString lines(int localLineNrFrom, int lineCount) const override;
    QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const override;
    bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind) override;

    QString selectedText() const override;
    QString positionLine() const override;

    void setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) override;
    void setPosToAbsStart(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) override;
    void setPosToAbsEnd(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) override;
    void selectAll() override;
    void clearSelection() override;
    QPoint position(bool local = false) const override;
    QPoint anchor(bool local = false) const override;
    bool hasSelection() const override;
    int selectionSize() const override;

    bool atTail() override;
    void updateSearchSelection() override;
    void clearSearchSelection() override;

    QPoint searchSelectionStart() override;
    QPoint searchSelectionEnd() override;
    void dumpPos() const override;
    qint64 checkField(Field field) const;

    void setOverscanLines(int newOverscanLines);

public slots:
    void reset() override;

private slots:
    void closeFile();                                           //2FF
    void closeAndReset();

private:
    enum PosAncState {PosAfterAnc, PosEqualAnc, PosBeforeAnc};

private:
    QList<qint64> scanLF(QList<qint64> &lf);
    QPoint endPosition();
    bool adjustLines(int &lineNr, int &count) const;
    void initDelimiter() const;
    bool reload();
    PosAncState posAncState() const;
    bool isBefore(const QPoint &textPos1, const QPoint &textPos2);

private:
    mutable QFile mFile;            // mutable to provide consistant logical const-correctness
    qint64 mSize = 0;
    QList<qint64> mLineByte;        // the n-th byte where each line starts (in contrast to the n-th char, see UTF-8)
    mutable QMutex mMutex;
    LinesCache mCache;
    int mOverscanLines = 50;
    int mVisibleTopLine = -1;
    int mCursorColumn = 0;
    QPoint mPosition;
    QPoint mAnchor;
    QPoint mSearchPos;
    QPoint mSearchEndPos;
    QPoint mSearchSelectionStart;
    QPoint mSearchSelectionEnd;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FASTFILEMAPPER_H
