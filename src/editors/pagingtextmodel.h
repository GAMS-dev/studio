#ifndef PAGINGTEXTMODEL_H
#define PAGINGTEXTMODEL_H

#include <QAbstractTableModel>
#include <QTextCodec>
#include <QFontMetrics>
#include "syntax.h"

namespace gams {
namespace studio {


class TextProvider
{
public:
    virtual ~TextProvider() {}
    virtual void openFile(const QString &fileName) = 0;
    virtual void clear() = 0;
    virtual int blockCount() const = 0;
    virtual QString block(int i) const = 0;
    virtual void appendLine(const QString &line) {Q_UNUSED(line)}
    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);

protected:
    QTextCodec *mCodec = nullptr;
};

///
/// class RawText
/// Reads a whole file into a QByteArray and uses indexes to build the lines for the model on the fly.
///
class RawText: public TextProvider
{
public:
    explicit RawText();
    ~RawText() override;
    void openFile(const QString &fileName) override;
    void clear() override;
    int blockCount() const override;
    QString block(int i) const override;
    void appendLine(const QString &block) override;

private:
    QByteArray mData;
    int mReserved = 0;
    QByteArray mDelimiter;
    QVector<int> mDataIndex;
};

///
/// class PagingText
/// Opens a file into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class PagingText: public TextProvider
{
    struct ChunkMap {qint64 start; qint64 size; uchar* pointer; QByteArray bArray; QVector<int> index; };
public:
    PagingText();
    ~PagingText() override;
    void openFile(const QString &fileName) override;
    void clear() override;
    int blockCount() const override;
    QString block(int i) const override;

private:
    ChunkMap &getChunk(qint64 byteNr);

private:
    int mChunkSize = 1000000;
    int mOverlap = 1000;
    int mMaxChunks = 5;
    QFile mFile;
    qint64 mFileSize;
    int mSizeDivisor;
    QVector<ChunkMap> mData;
    QByteArray mDelimiter;
    int mTopPos;
    int mPos;
    int mAnchor;
};

class PagingTextModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PagingTextModel(QFontMetrics metrics, QObject *parent = nullptr);
    ~PagingTextModel() override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void setLineNumbersVisible(bool visible);
    int columnWidth(int column);

signals:
    void reorganized();

public slots:
    void loadFile(const QString &fileName);
    void appendLine(const QString &line);

private:
    friend class PagingTextView;
    void updateSizes();
    void setFontMetrics(QFontMetrics metrics);

private:
    TextProvider *mTextData;
    bool mLineNumbersVisible = true;
    int mColumns = 2;
    int mLineNrWidth = 50;
    QList<TextMark*> mMarks;
    QFontMetrics mMetrics;
};

} // namespace studio
} // namespace gams

#endif // PAGINGTEXTMODEL_H
