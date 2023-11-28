#include "fastfilemapper.h"

#include <QRegularExpression>

namespace gams {
namespace studio {

FastFileMapper::FastFileMapper(QObject *parent)
    : gams::studio::AbstractTextMapper{parent}
{
    // TODO(JM) implement
}

FastFileMapper::~FastFileMapper()
{
    // TODO(JM) implement
}

AbstractTextMapper::Kind FastFileMapper::kind() const
{
    return AbstractTextMapper::fileMapper;
}

void FastFileMapper::startRun()
{
    // TODO(JM) implement
}

void FastFileMapper::endRun()
{
    // TODO(JM) implement
}

bool FastFileMapper::setVisibleTopLine(double region)
{
    // TODO(JM) implement
    return false;
}

bool FastFileMapper::setVisibleTopLine(int lineNr)
{
    // TODO(JM) implement
    return false;
}

int FastFileMapper::moveVisibleTopLine(int lineDelta)
{
    // TODO(JM) implement
    return 0;
}

int FastFileMapper::visibleTopLine() const
{
    // TODO(JM) implement
    return 0;
}

void FastFileMapper::scrollToPosition()
{
    // TODO(JM) implement
}

int FastFileMapper::lineCount() const
{
    // TODO(JM) implement
    return 0;
}

int FastFileMapper::knownLineNrs() const
{
    // TODO(JM) implement
    return 0;
}

QString FastFileMapper::lines(int localLineNrFrom, int lineCount) const
{
    // TODO(JM) implement
    return QString();
}

QString FastFileMapper::lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const
{
    // TODO(JM) implement
    return QString();
}

bool FastFileMapper::findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind)
{
    // TODO(JM) implement
    return false;
}

QString FastFileMapper::selectedText() const
{
    // TODO(JM) implement
    return QString();
}

QString FastFileMapper::positionLine() const
{
    // TODO(JM) implement
    return QString();
}

void FastFileMapper::setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode)
{
    // TODO(JM) implement
}

void FastFileMapper::setPosToAbsStart(QTextCursor::MoveMode mode)
{
    // TODO(JM) implement
}

void FastFileMapper::setPosToAbsEnd(QTextCursor::MoveMode mode)
{
    // TODO(JM) implement
}

void FastFileMapper::selectAll()
{
    // TODO(JM) implement
}

void FastFileMapper::clearSelection()
{
    // TODO(JM) implement
}

QPoint FastFileMapper::position(bool local) const
{
    // TODO(JM) implement
    return QPoint();
}

QPoint FastFileMapper::anchor(bool local) const
{
    // TODO(JM) implement
    return QPoint();
}

bool FastFileMapper::hasSelection() const
{
    // TODO(JM) implement
    return false;
}

int FastFileMapper::selectionSize() const
{
    // TODO(JM) implement
    return 0;
}

bool FastFileMapper::atTail()
{
    // TODO(JM) implement
    return false;
}

void FastFileMapper::updateSearchSelection()
{
    // TODO(JM) implement
}

void FastFileMapper::clearSearchSelection()
{
    // TODO(JM) implement
}

QPoint FastFileMapper::searchSelectionStart()
{
    // TODO(JM) implement
    return QPoint();
}

QPoint FastFileMapper::searchSelectionEnd()
{
    // TODO(JM) implement
    return QPoint();
}

void FastFileMapper::dumpPos() const
{
    // TODO(JM) implement
}

void FastFileMapper::reset()
{
    // TODO(JM) implement
}

bool FastFileMapper::updateMaxTop()
{
    // TODO(JM) implement
    return false;
}

} // namespace studio
} // namespace gams
