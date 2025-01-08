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
#include <QGuiApplication>
#include <QClipboard>
#include <QtMath>
#include <QMutexLocker>

#include "abstracttextmapper.h"

namespace gams {
namespace studio {

AbstractTextMapper::AbstractTextMapper(QObject *parent): QObject(parent)
{
    mCodec = nullptr;
}

AbstractTextMapper::~AbstractTextMapper()
{}

QTextCodec *AbstractTextMapper::codec() const
{
    return mCodec;
}

void AbstractTextMapper::setCodec(QTextCodec *codec)
{
    mCodec = codec;
}

//void AbstractTextMapper::setEncoding(QStringConverter::Encoding encoding)
//{
//    mEncoding = encoding;
//    encode = QStringEncoder(encoding); // QStringEncoder::Flag::Default
//    decode = QStringDecoder(encoding); // QStringEncoder::Flag::Default
//}

bool AbstractTextMapper::isEmpty() const
{
    return size() == 0;
}

void AbstractTextMapper::reset()
{
    mDelimiter.clear();
    mCursorColumn = 0;
}

qint64 AbstractTextMapper::size() const
{
    return 0;
}

QByteArray AbstractTextMapper::delimiter() const
{
    QMutexLocker locker(&mMutex);
    return mDelimiter;
}

void AbstractTextMapper::setVisibleLineCount(int visibleLines)
{
    mVisibleLineCount = qMax(1, visibleLines);
}

int AbstractTextMapper::visibleLineCount() const
{
    return mVisibleLineCount;
}

int AbstractTextMapper::reducedVisibleLineCount()
{
    return qCeil(visibleLineCount() * 0.95);
}

void AbstractTextMapper::setLineMarkers(const QList<int> &lines)
{
    mLineMarkers = lines;
}

QList<int> AbstractTextMapper::lineMarkers() const
{
    return mLineMarkers;
}

void AbstractTextMapper::setSearchSelectionActive(bool active)
{
    mIsSearchSelectionActive = active;
}

bool AbstractTextMapper::hasSearchSelection()
{
    return mIsSearchSelectionActive;
}

void AbstractTextMapper::copyToClipboard()
{
    QString text = selectedText();
    if (!text.isEmpty()) {
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setText(text);
    }
}

int AbstractTextMapper::maxLineWidth() const
{
    return mMaxLineWidth;
}

void AbstractTextMapper::setDelimiter(const QByteArray &delim) const
{
    QMutexLocker locker(&mMutex);
    mDelimiter = delim;
}

void AbstractTextMapper::setDebugMode(bool debug)
{
    mDebugMode = debug;
    updateMaxTop();
    emit blockCountChanged();
}


} // namespace studio
} // namespace gams
