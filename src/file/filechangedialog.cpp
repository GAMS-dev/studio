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
#include "filechangedialog.h"
#include <QKeyEvent>

namespace gams {
namespace studio {

FileChangeDialog::FileChangeDialog(QWidget *parent) :
    QMessageBox(parent)
{
    mCbAll = new QCheckBox("For &all files");
    mCbAll->setObjectName("cbAll");
    setCheckBox(mCbAll);
    mButtons << new QPushButton("Close") << new QPushButton("Reload") << new QPushButton("Always Reload") << new QPushButton("Keep");
    mButtons.at(int(Result::rClose))->setObjectName("btClose");
    mButtons.at(int(Result::rReload))->setObjectName("btReload");
    mButtons.at(int(Result::rReloadAlways))->setObjectName("btReloadAlways");
    mButtons.at(int(Result::rKeep))->setObjectName("btKeep");
    mButtons.at(int(Result::rKeep))->setAutoDefault(true);
    setIcon(Warning);
    for (QAbstractButton *button : std::as_const(mButtons)) {
        addButton(button, AcceptRole);
        connect(button, &QAbstractButton::clicked, this, &FileChangeDialog::buttonClicked);
    }
}

FileChangeDialog::~FileChangeDialog()
{}

void FileChangeDialog::show(const QString &filePath, bool deleted, bool modified, int count)
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


