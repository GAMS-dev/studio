/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "statuswidgets.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <logger.h>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QLayout>
#include <QStyleOption>
#include <QStringDecoder>

namespace gams {
namespace studio {

StatusWidgets::StatusWidgets(QMainWindow *parent) : QObject(parent), mStatusBar(parent->statusBar())
{
    mEditLines = new QLabel("0 lines");
    mStatusBar->addPermanentWidget(mEditLines);
    mEditLines->setMinimumWidth(mEditLines->height()*2);
    mEditLines->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mEditPosAnsSel = new QLabel("0 / 0");
    mStatusBar->addPermanentWidget(mEditPosAnsSel);
    mEditPosAnsSel->setMinimumWidth(mEditPosAnsSel->height()*2);
    mEditPosAnsSel->setAlignment(Qt::AlignCenter);

    mEditMode = new QLabel("INS");
    mStatusBar->addPermanentWidget(mEditMode);
    mEditMode->setMinimumWidth(int(mEditMode->height()*0.8));
    mEditMode->setAlignment(Qt::AlignCenter);

    mEditEncode = new QLabel("");
    mStatusBar->addPermanentWidget(mEditEncode);
    mEditEncode->setMinimumWidth(mEditEncode->height()*3);

    mFileName = new AmountLabel("Filename");
    mFileName->setLoadingText("(counting)");
    mStatusBar->addWidget(mFileName, 1);
}

void StatusWidgets::setFileName(const QString &fileName)
{
    mFileName->setAmount(1.0);
    mLoadAmount = 1.0;
    mFileName->setBaseText(fileName);
}

void StatusWidgets::setEncoding(const QString &encodingName)
{
    mEditEncode->setText(encodingName);
}

void StatusWidgets::setLineCount(int lines)
{
    if (lines < -1) {
        mEditLines->setText(QString("~%1 lines").arg(-lines));
    } else if (lines >= 0) {
        mEditLines->setText(QString("%1 lines").arg(lines));
    } else {
        mEditLines->setText("            ");
    }
}

void StatusWidgets::setLoadAmount(qreal amount)
{
    mFileName->setAmount(amount);
}

void StatusWidgets::setEditMode(EditMode mode)
{
    switch (mode) {
    case EditMode::Readonly: mEditMode->setText("RO"); break;
    case EditMode::Insert: mEditMode->setText("INS"); break;
    case EditMode::Overwrite: mEditMode->setText("OVR"); break;
    }
}

void StatusWidgets::setPosAndAnchor(QPoint pos, QPoint anchor)
{
    QString posText;
    if (pos.isNull() || pos.y() == -1) {
        posText = "      ";
    } else {
        QString estimated = (pos.y() < 0) ? "~" : "";
        posText = QString("%1%2 / %3").arg(estimated).arg(qAbs(pos.y())).arg(pos.x());
        if (!anchor.isNull() && anchor != pos) {
            estimated = (pos.y()<0 || anchor.y()<0) ? "~" : "";
            QString absLineDiff = QString::number(qAbs(qAbs(pos.y())-qAbs(anchor.y()))+1);
            posText += QString(" (%1%2 / %3)").arg(estimated, absLineDiff).arg(qAbs(pos.x()-anchor.x()));
        }
    }
    mEditPosAnsSel->setText(posText);
}

void StatusWidgets::setLoadingText(const QString &loadingText)
{
    mFileName->setLoadingText(loadingText);
}

void AmountLabel::setAmount(qreal value)
{
    if ((value < 1.0) != (mLoadAmount < 1.0)) {
        mLoadAmount = value;
        setText(mBaseText + ((mLoadAmount < 1.0) ? " "+mLoadingText : ""));
    } else {
        mLoadAmount = value;
        repaint();
    }
}

void AmountLabel::setBaseText(const QString &text)
{
    mBaseText = text;
    setText(mBaseText + ((mLoadAmount < 1.0) ? " "+mLoadingText : ""));
}

void AmountLabel::setLoadingText(const QString &loadingText)
{
    mLoadingText = loadingText;
    setText(mBaseText + ((mLoadAmount < 1.0) ? " "+mLoadingText : ""));
}

void AmountLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (mLoadAmount < 1.0) {
        int x = qRound((width() - 1) * qBound(0.0 ,mLoadAmount, 1.0));
        QPainter p(this);
        p.save();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(160,160,160, 220));
        p.drawRect(QRect(x, 0, width()-1, 3));
        p.setBrush(QColor(255,120,0, 220));
        p.drawRect(QRect(0, 0, x, 3));
        p.restore();
    }
}

} // namespace Studio
} // namespace gams
