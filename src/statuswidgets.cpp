#include "statuswidgets.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QTextCodec>

namespace gams {
namespace studio {

StatusWidgets::StatusWidgets(QMainWindow *parent) : QObject(parent), mStatusBar(parent->statusBar())
{
    mEditLines = new QLabel("0 lines ");
    mStatusBar->addPermanentWidget(mEditLines);
    mEditLines->setMinimumWidth(mEditLines->height()*2);
    mEditLines->setAlignment(Qt::AlignRight);

    mEditPosAnsSel = new QLabel(" 0 / 0 ");
    mStatusBar->addPermanentWidget(mEditPosAnsSel);
    mEditPosAnsSel->setMinimumWidth(mEditPosAnsSel->height()*2);
    mEditPosAnsSel->setAlignment(Qt::AlignCenter);

    mEditMode = new QLabel("INS");
    mStatusBar->addPermanentWidget(mEditMode);
    mEditMode->setMinimumWidth(mEditMode->height()*0.8);
    mEditMode->setAlignment(Qt::AlignCenter);

    mEditEncode = new QLabel("x");
    mStatusBar->addPermanentWidget(mEditEncode);
    mEditEncode->setMinimumWidth(mEditEncode->height()*3);

    mFileName = new QLabel("Filename");
    mStatusBar->addWidget(mFileName, 1);
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
        mEditLines->setText("");
    } else {
        mEditLines->setText(QString("%1 lines ").arg(lines));
    }
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
    if (pos.isNull()) {
        posText = "     ";
    } else {
        posText = QString("%1 / %2").arg(pos.y()).arg(pos.x());
        if (!anchor.isNull() && anchor != pos) {
            posText += QString(" (%1 / %2)").arg(qAbs(pos.y()-anchor.y()+1)).arg(qAbs(pos.x()-anchor.x()));
        }
    }
    mEditPosAnsSel->setText(posText);
}

} // namespace Studio
} // namespace gams
