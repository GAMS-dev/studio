#include "statuswidgets.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QTextCodec>
#include <logger.h>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QLayout>
#include <QStyleOption>

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
    mStatusBar->addWidget(mFileName, 1);
}

void StatusWidgets::setFileName(const QString &fileName)
{
    mFileName->setText(fileName);
    mFileName->setAmount(1.0);
    mLoadAmount = 1.0;
}

void StatusWidgets::setEncoding(int encodingMib)
{
    if (encodingMib == -1) {
        mEditEncode->setText("");
    } else {
        QTextCodec* codec = QTextCodec::codecForMib(encodingMib);
        mEditEncode->setText(codec->name());
    }
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
            posText += QString(" (%1%2 / %3)").arg(estimated).arg(absLineDiff).arg(qAbs(pos.x()-anchor.x()));
        }
    }
    mEditPosAnsSel->setText(posText);
}

void AmountLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (mLoadAmount < 1.0) {
        QPainter p(this);
        p.save();
        p.setBrush(QColor(255,120,0, 220));
        p.setPen(Qt::NoPen);
        int x = qRound((width() - 1) * qBound(0.0 ,mLoadAmount, 1.0));
        p.drawRect(QRect(0, 0, x, 3));
        p.restore();
    }
}

} // namespace Studio
} // namespace gams
