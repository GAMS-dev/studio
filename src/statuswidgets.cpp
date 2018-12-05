#include "statuswidgets.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QTextCodec>
#include <logger.h>
#include <QPainter>
#include <QStyle>
#include <QLayout>
#include <QStyleOption>

namespace gams {
namespace studio {

StatusWidgets::StatusWidgets(QMainWindow *parent) : QObject(parent), mStatusBar(parent->statusBar())
{
    mEditLines = new QLabel("0 lines ");
    mStatusBar->addPermanentWidget(mEditLines);
    mEditLines->setMinimumWidth(mEditLines->height()*2);
    mEditLines->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mEditLines->setAutoFillBackground(true);

    mEditPosAnsSel = new QLabel(" 0 / 0 ");
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

    mFileName = new QLabel("Filename");
    mStatusBar->addWidget(mFileName, 1);
    mFileName->setAutoFillBackground(true);
}

void StatusWidgets::setFileName(const QString &fileName)
{
    mFileName->setText(fileName);
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
    if (lines < 0) {
        mEditLines->setText(QString("~%1 lines ").arg(-lines));
    } else {
        mEditLines->setText(QString("%1 lines ").arg(lines));
    }
}

void StatusWidgets::setLoadAmount(qreal amount)
{
    QLabel *la = mFileName;
    QPalette pal = la->palette();
    QPixmap pix(la->size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setBrush(QColor(100,150,200, 100));
    p.setPen(Qt::NoPen);
    int x = qRound(la->width() * qBound(0.0 ,amount, 1.0));
    if (amount < 1.0)
        p.drawRect(QRect(0, 0, x, la->height()));
//    p.drawRect(QRect(x, 0, la->width()-x, la->height()));
    pal.setBrush(QPalette::Background, QBrush(pix));
    la->setPalette(pal);
    la->repaint();
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
        posText = QString("%3%1 / %2").arg(qAbs(pos.y())).arg(pos.x()).arg(estimated);
        if (!anchor.isNull() && anchor != pos) {
            estimated = (pos.y()<0 || anchor.y()<0) ? "~" : "";
            QString absLineDiff = QString::number(qAbs(qAbs(pos.y())-qAbs(anchor.y()))+1);
            posText += QString(" (%3%1 / %2)").arg(absLineDiff).arg(qAbs(pos.x()-anchor.x())).arg(estimated);
        }
    }
    mEditPosAnsSel->setText(posText);
}

//void AmountLabel::paintEvent(QPaintEvent *event)
//{
//    QLabel::paintEvent(event);
//    QStyleOption opt;
//    opt.initFrom(this);
//    QBrush
//    opt.

//    QStyle *style = QWidget::style();
//    QPainter painter(this);
//    drawFrame(&painter);
//    QRect cr = contentsRect();
//    cr.adjust(margin(), margin(), -margin(), -margin());
//    int align = QStyle::visualAlignment(layoutDirection(), alignment());
//    QRectF lr = d->layoutRect().toAlignedRect();
//}





} // namespace Studio
} // namespace gams
