#include "filechangedialog.h"
#include "../logger.h"
#include <QKeyEvent>

namespace gams {
namespace studio {

FileChangeDialog::FileChangeDialog(QWidget *parent) :
    QMessageBox(parent)
{
    mCbAll = new QCheckBox("For &all files");
    setCheckBox(mCbAll);
    mButtons << new QPushButton("Close") << new QPushButton("Reload") << new QPushButton("Always Reload") << new QPushButton("Keep");
    mButtons.at(int(Result::rKeep))->setAutoDefault(true);
    setIcon(Warning);
    for (QAbstractButton *button : qAsConst(mButtons)) {
        addButton(button, AcceptRole);
        connect(button, &QAbstractButton::clicked, this, &FileChangeDialog::buttonClicked);
    }
}

FileChangeDialog::~FileChangeDialog()
{}

void FileChangeDialog::show(QString filePath, bool deleted, bool modified, int count)
{
    mCbAll->setChecked(false);
    setWindowTitle(QString("File %1").arg(deleted ? "vanished" : "changed"));
    QString file("\"" + filePath + "\"");
    QString text(file + (deleted ? "%1 doesn't exist anymore."
                                 : (count>1 ? "%1 have been modified externally."
                                            : "%1 has been modified externally.")));
    text = text.arg(count<2? "" : QString(" and %1 other file%2").arg(count-1).arg(count<3? "" : "s"));
    text += "\nDo you want to %1?";
    if (deleted) text = text.arg("keep the file in editor");
    else if (modified) text = text.arg("reload the file or keep your changes");
    else text = text.arg("reload the file");
    setText(text);
    mButtons.at(int(Result::rClose))->setVisible(deleted);
    mButtons.at(int(Result::rReload))->setVisible(!deleted);
    mButtons.at(int(Result::rReloadAlways))->setVisible(!deleted);
    mCbAll->setVisible(count > 1);
    DEB() << "open for " << file << (count > 1 ? QString(" + %1").arg(count-1) : "");
    QMessageBox::open();
}

bool FileChangeDialog::isForAll()
{
    return mCbAll->isChecked();
}

void FileChangeDialog::keyPressEvent(QKeyEvent *event)
{
    if (mCbAll->isVisible() && event->key() == Qt::Key_A) mCbAll->toggle();
    QDialog::keyPressEvent(event);
}

void FileChangeDialog::buttonClicked()
{
    setResult(mButtons.indexOf(static_cast<QPushButton*>(sender())) + (isForAll() ? 4 : 0));
    emit ready(result());
    QMessageBox::close();
}

} // namespace studio
} // namespace gams


