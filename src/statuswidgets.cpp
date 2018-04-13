#include "statuswidgets.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>

namespace gams {
namespace studio {

StatusWidgets::StatusWidgets(QMainWindow *parent) : QObject(parent), mStatusBar(parent->statusBar())
{
    mEditPosAnsSel = new QLabel("0 / 0");
    mStatusBar->addPermanentWidget(mEditPosAnsSel);
    mEditMode = new QLabel("INS");
    mStatusBar->addPermanentWidget(mEditMode);
    mFileName = new QLabel("Filename");
    mStatusBar->addWidget(mFileName);
}

void StatusWidgets::setFileName(const QString &fileName)
{
    mFileName->setText(fileName);
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
