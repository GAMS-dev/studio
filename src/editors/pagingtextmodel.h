#ifndef PAGINGTEXTMODEL_H
#define PAGINGTEXTMODEL_H

#include <QAbstractTableModel>
#include <QTextCodec>
#include <QTableView>
#include "syntax.h"

namespace gams {
namespace studio {

class RawText
{
public:
    explicit RawText();
    void loadFile(const QString &fileName);
    void clear();
    int lineCount() const;
    QString line(int i) const;
    void appendLine(const QString &line);

private:
    QTextCodec *mCodec = nullptr;
    QByteArray mData;
    int mReserved = 0;
    QByteArray mDelimiter;
    QVector<int> mDataIndex;
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
    RawText mRawText;
    bool mLineNumbersVisible = true;
    int mColumns = 2;
    int mLineNrWidth = 50;
    QList<TextMark*> mMarks;
    QFontMetrics mMetrics;
};

class PagingTextView: public QTableView
{
    Q_OBJECT
public:
    explicit  PagingTextView(QWidget *parent);

    void loadFile(QString fileName);

    FileId fileId() const;
    void setFileId(const FileId &fileId);
    NodeId groupId() const;
    virtual void setGroupId(const NodeId &groupId);

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    void zoomInF(float range);

public slots:
    void reorganize();

protected:
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

private:
    PagingTextModel mModel;
    FileId mFileId;
    NodeId mGroupId;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;
};

} // namespace studio
} // namespace gams

#endif // PAGINGTEXTMODEL_H
