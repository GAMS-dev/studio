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
#ifndef ABSTRACTTEXTMAPPER_H
#define ABSTRACTTEXTMAPPER_H

#include <QObject>
#include <QVector>
#include <QSet>
#include <QTextCursor>
#include <QTextDocument>
#include <QPoint>
#include <QMutex>

#include "search/result.h"

class QStringDecoder;

namespace gams {
namespace studio {

struct LineFormat {
    LineFormat() {}
    LineFormat(const LineFormat &other) { *this = other; }
    LineFormat(int _start, int _end, const QTextCharFormat &_format)
        : start(_start), end(_end), format(_format) {}
    LineFormat(int _start, int _end, const QTextCharFormat &_format, const QString &ref)
        : start(_start), end(_end), format(_format) {
        format.setAnchorHref(ref);
    }
    LineFormat &operator=(const LineFormat &other) {
        start = other.start; end = other.end; format = other.format;
        extraLstFormat = other.extraLstFormat; extraLstHRef = other.extraLstHRef; lineMarked = other.lineMarked;
        return *this;
    }
    bool operator ==(const LineFormat &other) const {
        return start == other.start && end == other.end && format == other.format &&
            extraLstFormat == other.extraLstFormat && extraLstHRef == other.extraLstHRef && lineMarked == other.lineMarked;
    }
    int start = -1;
    int end = -1;
    QTextCharFormat format;
    const QTextCharFormat *extraLstFormat = nullptr;
    QString extraLstHRef;
    bool lineMarked = false;
};

class FileMeta;
///
/// class AbstractTextMapper
/// Maps text data into chunks of QByteArrays that are loaded on request. Uses indexes to build the lines for the
/// model on the fly.
///
class AbstractTextMapper: public QObject
{
    Q_OBJECT
public:
    enum Kind {fileMapper, memoryMapper};
    enum SpecialCursorPosition { cursorInvalid = -1, cursorBeforeStart = -2, cursorBeyondEnd = -3 };

public:
    ~AbstractTextMapper() override;
    virtual AbstractTextMapper::Kind kind() const = 0;
    virtual void startRun() = 0;
    virtual void endRun() = 0;

    QString encoding();
    void setEncoding(const QString &encoding);
    bool isEmpty() const;
    virtual qint64 size() const;
    virtual QByteArray delimiter() const;

    virtual void setVisibleLineCount(int visibleLines);
    virtual int visibleLineCount() const;
    int reducedVisibleLineCount();
    virtual bool setVisibleTopLine(double region) = 0;
    virtual bool setVisibleTopLine(int lineNr) = 0;
    virtual int moveVisibleTopLine(int lineDelta) = 0;
    virtual int visibleTopLine() const = 0;
    virtual void scrollToPosition() = 0;

    virtual int lineCount() const = 0;
    virtual int knownLineNrs() const = 0;

    virtual QString lines(int localLineNrFrom, int lineCount) const = 0;
    virtual QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const = 0;
    virtual bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind) = 0;

    virtual QString selectedText() const = 0;
    virtual QString positionLine() const = 0;
    virtual void copyToClipboard();

    virtual void setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) = 0;
    virtual void setPosToAbsStart(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) = 0;
    virtual void setPosToAbsEnd(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor) = 0;
    virtual void selectAll() = 0;
    virtual void clearSelection() = 0;
    virtual QPoint position(bool local = false) const = 0;
    virtual QPoint anchor(bool local = false) const = 0;
    virtual bool hasSelection() const = 0;
    virtual int selectionSize() const = 0;
    virtual void setDebugMode(bool debug);
    bool debugMode() const { return mDebugMode; }
    virtual bool atTail() = 0;
    virtual void updateSearchSelection() = 0;
    virtual void clearSearchSelection() = 0;
    void setSearchSelectionActive(bool active);
    bool hasSearchSelection();
    virtual QPoint searchSelectionStart() = 0;
    virtual QPoint searchSelectionEnd() = 0;
    void setLineMarkers(const QList<int> &lines);
    QList<int> lineMarkers() const;

    virtual void dumpPos() const = 0;

public slots:
    virtual void reset() = 0;

signals:
    void blockCountChanged();
    void loadAmountChanged(int knownLineCount);
    void selectionChanged();

protected:
    AbstractTextMapper(QObject *parent = nullptr);
    int maxLineWidth() const;
    virtual bool updateMaxTop() { return true; }
    virtual void setDelimiter(const QByteArray &delim) const;
    QByteArray decode(const QByteArray &data) const;
    QByteArray encode(const QString &data) const;

private:
    mutable QByteArray mDelimiter;
    mutable QMutex mMutex;

    bool mIsSearchSelectionActive = false;
    QList<int> mLineMarkers;
    int mVisibleLineCount = 0;
    int mCursorColumn = 0;
    int mMaxLineWidth = 1024*512;
    bool mDebugMode = false;

    mutable QStringDecoder mDecoder;
    mutable QStringEncoder mEncoder;
};

} // namespace studio
} // namespace gams

#endif // ABSTRACTTEXTMAPPER_H
