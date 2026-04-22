/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include "scrollmarks.h"
#include "logger.h"

#include <QStyleOption>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QPainter>
#include <QEvent>
#include <QTextBlock>
#include <QTimer>

namespace gams {
namespace studio {

ScrollMarks::ScrollMarks(QPlainTextEdit *parent) : QWidget(parent->verticalScrollBar()), mEdit(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    if (mEdit->verticalScrollBar()) {
        mEdit->installEventFilter(this);
        mEdit->verticalScrollBar()->installEventFilter(this);
        connect(mEdit->document(), &QTextDocument::contentsChanged, this, QOverload<>::of(&ScrollMarks::update));
    }
    QTimer::singleShot(0, this, &ScrollMarks::updateGeometry);
}

void ScrollMarks::setMarks(const QColor &color, const QList<int> &lines)
{
    if (lines.isEmpty()) {
        clearMarks(color);
    } else {
        mMarks[color.rgba()] = lines;
        update();
    }
}

void ScrollMarks::clearMarks(QColor color)
{
    mMarks.remove(color.rgba());
    update();
}

void ScrollMarks::clearMarks()
{
    mMarks.clear();
    update();
}

bool ScrollMarks::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == mEdit->verticalScrollBar()) {
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide ||
            event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            QWidget *wid = parentWidget();
            if (event->type() == QEvent::Resize) {
                QResizeEvent *eResize = static_cast<QResizeEvent*>(event);
                wid = eResize->size().width()>0 && mEdit->verticalScrollBar()->isVisible() ?
                          static_cast<QWidget*>(mEdit->verticalScrollBar()) : mEdit;
            } else if (event->type() == QEvent::Show) {
                wid = mEdit->verticalScrollBar();
            } else if (event->type() == QEvent::Hide) {
                wid = mEdit;
            }
            if (wid != parentWidget()) {
                setParent(wid);
                setAttribute(Qt::WA_TransparentForMouseEvents);
                setAttribute(Qt::WA_NoSystemBackground);
            }
            updateGeometry();
        }
    }
    else if (obj == mEdit) {
        if (event->type() == QEvent::Resize) {
            updateGeometry();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ScrollMarks::paintEvent(QPaintEvent *)
{
    if (!mEdit || mMarks.isEmpty()) return;

    QPainter painter(this);
    int w = width();
    int h = height();

    QScrollBar* sb = mEdit->verticalScrollBar();
    bool sbVisible = sb->isVisible();
    if (!sbVisible || sb->width() < w)
        painter.fillRect(rect(), QColor(200, 200, 200, 30));

    painter.setOpacity(sbVisible ? 0.7 : 0.9);

    const qsizetype totalBlocks = mEdit->blockCount();
    if (totalBlocks <= 1) return;
    double scale = static_cast<double>(h) / static_cast<double>(totalBlocks);
    auto* layout = qobject_cast<QPlainTextDocumentLayout*>(mEdit->document()->documentLayout());
    if (!layout) return;

    QHashIterator<QRgb, QList<int>> i(mMarks);
    while (i.hasNext()) {
        i.next();
        QColor color = QColor::fromRgba(i.key());
        const QList<int>& lines = i.value();
        int lastY = -1;

        for (qsizetype line : lines) {
            int y = static_cast<int>(line * scale);

            if (y != lastY && y >= 0 && y < h) {
                painter.fillRect(0, y, w, 2, color);
                lastY = y;
            }
        }
    }
}

void ScrollMarks::updateGeometry()
{
    QScrollBar* sb = mEdit->verticalScrollBar();
    if (!sb) return;

    QRect rect;
    if (parentWidget() == sb) {
        QStyleOptionSlider opt;
        opt.initFrom(sb);
        opt.orientation = Qt::Vertical;
        rect = sb->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, sb);
        if (rect.isEmpty()) rect = sb->geometry();
    } else {
        rect = mEdit->geometry();
        rect.setLeft( rect.right() - 10);
    }
    setGeometry(rect);
    show();
    raise();
}



} // namespace studio
} // namespace gams
