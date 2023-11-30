#ifndef GAMS_STUDIO_FASTFILEMAPPER_H
#define GAMS_STUDIO_FASTFILEMAPPER_H

#include "abstracttextmapper.h"
#include <QFile>
#include <QMutex>

namespace gams {
namespace studio {

class FastFileMapper : public AbstractTextMapper
{
    Q_OBJECT
public:
    explicit FastFileMapper(QObject *parent = nullptr);
    ~FastFileMapper() override;
    virtual AbstractTextMapper::Kind kind() const;

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

public slots:
    void reset() override;

private slots:
    void closeFile();                                           //2FF
    void closeAndReset();

private:
    enum PosAncState {PosAfterAnc, PosEqualAnc, PosBeforeAnc};

private:
    QList<qint64> scanLF();
    QPoint endPosition();
    QString readLines(int lineNr, int count) const;
    bool adjustLines(int &lineNr, int &count) const;
    void initDelimiter() const;
    bool reload();
    PosAncState posAncState() const;

private:
    mutable QFile mFile;                // mutable to provide consistant logical const-correctness
    qint64 mSize = 0;
    QList<qint64> mLines;
    QMutex mMutex;
    int mVisibleTopLine = -1;
    int mVisibleLineCount = 0;
    QPoint mPosition;
    QPoint mAnchor;
    QPoint mSearchSelectionStart;
    QPoint mSearchSelectionEnd;

};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FASTFILEMAPPER_H
