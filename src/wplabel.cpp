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
#include "wplabel.h"
#include "mainwindow.h"
#include <QDesktopServices>

namespace gams {
namespace studio {

WpLabel::WpLabel(QWidget *parent) : QLabel(parent)
{
    setStyleSheet("QLabel { background-color : white; }");
}

WpLabel::WpLabel(const QString &content, const QString &link, QWidget *parent)
    : QLabel(parent), mContent(content), mLink(link)
{
    QLabel::setText(mContent);
    setStyleSheet("QLabel { background-color : white; }");
    setFrameShape(QFrame::StyledPanel);
    setMargin(4);
    setWordWrap(true);
}

void WpLabel::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (mInactive) return;
    setFrameShape(QFrame::Box);
    setStyleSheet("QLabel { background-color : #f39619; }");
}

void WpLabel::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (mInactive) return;
    setFrameShape(QFrame::StyledPanel);
    setStyleSheet("QLabel { background-color : white; }");
}

void WpLabel::setInactive(bool inactive)
{
    mInactive = inactive;
}

void WpLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (mInactive || event->button() == Qt::RightButton) return;

    if (!mLink.isNull()) { // file history
        QLabel::linkActivated(mLink);

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

}
}
