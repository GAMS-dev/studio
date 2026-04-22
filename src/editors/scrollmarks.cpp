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
#include "qstyleoption.h"

#include <QPlainTextEdit>
#include <QScrollBar>
#include <QPainter>
#include <QEvent>
#include <QTextBlock>

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
    updateGeometry();
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
            if (mEdit->verticalScrollBar()->isVisible())
                setParent(mEdit->verticalScrollBar());
            else
                setParent(mEdit->viewport());
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

    bool sbVisible = mEdit->verticalScrollBar()->isVisible();
    if (!sbVisible || mEdit->verticalScrollBar()->width() < w)
        painter.fillRect(rect(), QColor(200, 200, 200, 30));

    painter.setOpacity(sbVisible ? 0.7 : 0.9);

    const qsizetype totalBlocks = mEdit->blockCount();
    if (totalBlocks <= 1) return;
    double scale = static_cast<double>(h) / static_cast<double>(totalBlocks);

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

    if (sb->maximum() > 0) {

        QStyleOptionSlider opt;
        opt.initFrom(sb);
        opt.orientation = Qt::Vertical;
        QRect rect = sb->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, sb);
        if (rect.isEmpty())
            rect = sb->geometry();

        int w = sb->isVisible() ? sb->width() - 2 : 10;
        int h = rect.height();
        int x = sb->isVisible() ? rect.left() + 1 : (mEdit->width() - w - 1);

        if (sb->isVisible() && rect.width() < w)
            x -= (w - rect.width());

        setGeometry(x, rect.top(), w, h);
        show();
        raise();
    } else {
        hide();
    }
}



} // namespace studio
} // namespace gams
