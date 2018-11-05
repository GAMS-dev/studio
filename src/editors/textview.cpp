/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#include "textview.h"
#include "logger.h"
#include "exception.h"

#include <QScrollBar>
#include <QTextBlock>
#include <QPlainTextDocumentLayout>
#include <QBoxLayout>

namespace gams {
namespace studio {


TextView::TextView(QWidget *parent) : QAbstractScrollArea(parent)
{
    setViewportMargins(0,0,0,0);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mEdit = new TextViewEdit(this);
    mEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,18,0);
    setLayout(lay);
    lay->addWidget(mEdit);
    connect(mEdit, &CodeEdit::cursorPositionChanged, this, &TextView::editScrollChanged);
    connect(mEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextView::editScrollChanged);
    connect(mEdit->verticalScrollBar(), &QScrollBar::rangeChanged, this, &TextView::editScrollResized);
//    mEdit->verticalScrollBar()->setVisible(false);

/* --- scrollbar controlling qt-methods
    QObject::connect(control, SIGNAL(documentSizeChanged(QSizeF)), q, SLOT(_q_adjustScrollbars()));
    QPlainTextEdit::setDocument(QTextDocument *document);
    QPlainTextEditPrivate::append(const QString &text, Qt::TextFormat format);
    QPlainTextEdit::resizeEvent(QResizeEvent *e);
    QPlainTextEdit::setLineWrapMode(LineWrapMode wrap);
*/

}

int TextView::lineCount()
{
    if (!mEdit->document()) return 0;
    return mEdit->document()->lineCount();
}

void TextView::loadFile(const QString &fileName, QList<int> codecMibs)
{
//    mData->openFile(fileName);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        EXCEPT() << "Error opening file " << fileName;
    const QByteArray data(file.readAll());
    mEdit->setupFileSize(file.size());

    QTextCodec *codec = nullptr;
    QList<int> mibs = codecMibs;
    mibs << QTextCodec::codecForLocale()->mibEnum();
    for (int mib: mibs) {
        QTextCodec::ConverterState state;
        codec = QTextCodec::codecForMib(mib);
        if (codec) {
            QString text = codec->toUnicode(data.constData(), data.size(), &state);
            if (state.invalidChars == 0) {
                mLoading = true;
                mEdit->document()->setPlainText(text);
                mLoading = false;
                mCodec = codec;
                break;
            }
        } else {
            DEB() << "System doesn't contain codec for MIB " << mib;
        }
    }
    file.close();
}

void TextView::zoomIn(int range)
{
    mEdit->zoomIn(range);
}

void TextView::zoomOut(int range)
{
    mEdit->zoomOut(range);
}

void TextView::editScrollChanged()
{
    TRACE();
    syncVScroll(mEdit->verticalScrollBar()->value(), -1);
}

void TextView::editScrollResized(int min, int max)
{
    TRACE();
    verticalScrollBar()->setMinimum(min);
    verticalScrollBar()->setMaximum(max);
}

void TextView::scrollContentsBy(int dx, int dy)
{
    TRACE();
    Q_UNUSED(dx)
    Q_UNUSED(dy)
    syncVScroll(-1, verticalScrollBar()->value());
}

void TextView::syncVScroll(int editValue, int mainValue)
{
    if (editValue >= 0) {
        int value = editValue;
        if (value != verticalScrollBar()->value())
            verticalScrollBar()->setValue(value);
    } else if (mainValue >= 0) {
        int value = mainValue;
        if (value != mEdit->verticalScrollBar()->value())
        mEdit->verticalScrollBar()->setValue(value);
    }
}


} // namespace studio
} // namespace gams
