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
#include "wplabel.h"
#include "mainwindow.h"
#include <QDesktopServices>
#include <QPainter>

namespace gams {
namespace studio {


WpLabel::WpLabel(QWidget *parent) : QLabel(parent)
{
    setAutoFillBackground(true);
    updateMouseOverColor(false);
}

WpLabel::WpLabel(const QString &content, const QString &link, QWidget *parent)
    : QLabel(parent), mContent(content), mLink(link)
{
    setFrameShape(QFrame::StyledPanel);
    setMargin(4);
    setWordWrap(true);
    setAutoFillBackground(true);
    updateMouseOverColor(false);

    QLabel::setText(mContent);
}

void WpLabel::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    if (mInactive) return;

    setFrameShape(QFrame::Box);
    updateMouseOverColor(true);
}

void WpLabel::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    if (mInactive) return;

    setFrameShape(QFrame::StyledPanel);
    updateMouseOverColor(false);
}

void WpLabel::updateMouseOverColor(bool hovered)
{
    QPoint pos = mapFromGlobal(QCursor::pos());
    mActive = hovered && !inCloseButton(pos);

    auto p = qApp->palette();
    p.setColor(QPalette::Window, mActive ? GAMS_ORANGE : p.color(QPalette::Base).lighter());
    setPalette(p);
}

bool WpLabel::inCloseButton(const QPoint &pos)
{
    if (!mCloseable) return false;
    int xSize = fontMetrics().height() / 2 + 2;
    return (pos.x() >= width() - xSize && pos.y() <= xSize);
}

void WpLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (mIcon.isNull()) return;

    QPainter painter(this);
    QRect rect = QRect(contentsRect().topLeft(), mIconSize);
    int cHei = contentsRect().height();
    if (!mAlignment.testFlag(Qt::AlignLeft))
            rect.moveLeft(mAlignment.testFlag(Qt::AlignRight) ? indent()-rect.width() : (indent()-rect.width())/2);
    if (!mAlignment.testFlag(Qt::AlignTop))
            rect.moveTop(mAlignment.testFlag(Qt::AlignBottom) ? cHei-rect.height() : (cHei-rect.height())/2);
    mIcon.paint(&painter, rect, mAlignment, mActive ? QIcon::Active : QIcon::Mode::Normal);
    if (mCloseable) {
        QPen pen = painter.pen();
        pen.setColor(Qt::gray);
        pen.setWidth(2);
        painter.setPen(pen);
        int xSize = fontMetrics().height() /2;
        QRect xRect(contentsRect().right() - 1 - xSize, contentsRect().top() + 2, xSize, xSize);
        painter.drawLine(xRect.topLeft(), xRect.bottomRight());
        painter.drawLine(xRect.topRight(), xRect.bottomLeft());
    }
}

void WpLabel::setInactive(bool inactive)
{
    mInactive = inactive;
}

void WpLabel::setCloseable(bool closeable)
{
    mCloseable = closeable;
}

void WpLabel::setIcon(const QIcon &icon)
{
    mIcon = icon;
    update();
}

void WpLabel::setIconSize(const QSize &size)
{
    mIconSize = size;
    if (indent() < mIconSize.width()) setIndent(mIconSize.width()+10);
    update();
}

void WpLabel::setIconAlignment(Qt::Alignment alignment)
{
    mAlignment = alignment;
    update();
}

void WpLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (mInactive || event->button() == Qt::RightButton) return;

    if (!mLink.isNull()) { // file history
        if (inCloseButton(event->pos()))
            emit removeFromHistory(mLink);
        else
            emit QLabel::linkActivated(mLink);

    // added via designer from here on
    } else if (!this->property("link").isNull()) { // web- or file links, open directly
        QString link = this->property("link").toString();
        QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));

    } else if (!this->property("action").isNull()) { // actions
        QString action = this->property("action").toString();
        emit relayActionLab(action);

    } else if (!this->property("modlib").isNull()) { // load item from model library
        QString lib = this->property("modlib").toString();
        emit relayModLibLoad(lib);
    } else if (!this->property("documentation").isNull()) { // open integrated documentation
        QString doc = this->property("documentation").toString();
        QString anchor = this->property("anchor").toString();
        emit relayOpenDoc(doc, anchor);
    }
}

void WpLabel::mouseMoveEvent(QMouseEvent *event)
{
    QLabel::mouseMoveEvent(event);
    updateMouseOverColor(true);
}

}
}
