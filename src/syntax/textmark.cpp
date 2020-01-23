/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "textmark.h"
#include "textmarkrepo.h"
#include "file.h"
#include "logger.h"
#include "exception.h"

namespace gams {
namespace studio {

TextMarkId TextMark::mNextId = 0;

TextMark::TextMark(TextMarkRepo *marks, FileId fileId, Type tmType, NodeId groupId)
    : mId(mNextId++), mFileId(fileId), mGroupId(groupId), mMarkRepo(marks), mType(tmType)
{
    Q_ASSERT_X(mMarkRepo, "TextMark constructor", "The TextMarkRepo must be a valid instance.");
}

void TextMark::setLine(int lineNr)
{
    mLine = lineNr;
}

TextMark *TextMark::refMark() const
{
    return mReference;
}

QVector<TextMark *> TextMark::backRefs(FileId fileId) const
{
    if (!fileId.isValid()) return mBackRefs;
    QVector<TextMark *> res;
    for (TextMark* mark: mBackRefs) {
        if (mark->fileId() == fileId) res << mark;
    }
    return res;
}

TextMark::~TextMark()
{
    clearBackRefs();
}

void TextMark::setPosition(int line, int column, int size)
{
    mLine = line;
    mSize = (size<0) ? -size : size;
    mColumn = (size<0) ? column-mSize : column;
}

void TextMark::jumpToRefMark(bool focus, bool ignoreColumn)
{
    if (mReference)
        mReference->jumpToMark(focus, ignoreColumn);
    else
        DEB() << "No TextMark reference to jump to";
}

void TextMark::jumpToMark(bool focus, bool ignoreColumn)
{
    mMarkRepo->jumpTo(this, focus, ignoreColumn);
}

void TextMark::setRefMark(TextMark* refMark)
{
    mReference = refMark;
    if (mReference)
        mReference->mBackRefs << this;
}

void TextMark::unsetRefMark(TextMark* refMark)
{
    if (mReference == refMark) mReference = nullptr;
    mBackRefs.removeAll(refMark);
}

void TextMark::clearBackRefs()
{
    if (mReference) mReference->unsetRefMark(this);
    for (TextMark* backRef: mBackRefs) {
        backRef->unsetRefMark(this);
    }
    mBackRefs.clear();
}

QColor TextMark::color() const
{
    if (mReference) {
        if (mReference->type() == TextMark::error) return Qt::darkRed;
        if (mReference->fileKind() == FileKind::Lst) return Qt::blue;
    } else {
        return Qt::darkRed;
    }
    return Qt::darkGreen;
}

FileKind TextMark::fileKind()
{
    return mMarkRepo->fileKind(mFileId);
}

FileKind TextMark::refFileKind()
{
    return mReference ? mReference->fileKind() : FileKind::None;
}

QIcon TextMark::icon()
{
    switch (mType) {
    case error:
        return QIcon(":/img/exclam-circle-r");
    case link:
        return mReference ? QIcon(":/img/err-ref") : QIcon(":/img/err-ref-missing");
    case bookmark: {
        QIcon ico(":/img/bookmark");
        return ico;
    }
    default:
        break;
    }
    return QIcon();
}



Qt::CursorShape& TextMark::cursorShape(Qt::CursorShape* shape, bool inIconRegion)
{
    if (shape && ((mType == error && inIconRegion) || mType == link))
        *shape = mReference ? Qt::PointingHandCursor : Qt::ForbiddenCursor;
    return *shape;
}

void TextMark::rehighlight()
{
    mMarkRepo->rehighlight(mFileId, mLine);
}

void TextMark::flatten()
{
    mSize = 0;
    mColumn = -1;
    rehighlight();
}

QString TextMark::dump()
{
    QStringList refs;
    for (TextMark* mark: mBackRefs) {
        refs << QString::number(mark->mId);
    }
    return QString("(%3,%4,%5)[%1%2] ").arg(mId)
            .arg(mReference ? "->"+QString::number(mReference->mId) : "")
            .arg(mLine).arg(mColumn).arg(mSize);
}

FileId TextMark::fileId() const
{
    return mFileId;
}

NodeId TextMark::groupId() const
{
    return mGroupId;
}

int TextMark::value() const
{
    return mValue;
}

void TextMark::setValue(int value)
{
    mValue = value;
}

} // namespace studio
} // namespace gams
