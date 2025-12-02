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
#include "pathrequest.h"
#include "projectrepo.h"
#include <QMessageBox>
#include <QPushButton>

namespace gams {
namespace studio {
namespace path {

PathRequest::PathRequest(QWidget *parent) :
    QObject(parent),
    mParent(parent)
{
}

PathRequest::~PathRequest()
{
}

void PathRequest::init(ProjectRepo *repo, const QString &baseDir, const QVariantMap &data)
{
    mData = data;
    mProjectRepo = repo;
    mBaseDir = QDir::toNativeSeparators(baseDir);
}

bool PathRequest::checkProject(const QString &gspName)
{
    int count;
    int ignored;
    QStringList missed;
    int ok = mProjectRepo->checkRead(mData, count, ignored, missed, mBaseDir);
    if (ok) {
        emit acceptOpen();
    } else {
        for (const QString &file : missed)
            emit warning("File not found: " + QDir::toNativeSeparators(file));
        bool one = (missed.size() == 1);
        mMsgBox = new QMessageBox(mParent);
        QString text = "Open project\n" + QDir::toNativeSeparators(gspName) +
                       "\n\n%1 referenced in the project could not be found at its relative path. You can\n"
                       "- copy the project files to the matching relative path\n"
                       "- ignore missing files and proceed";
        mMsgBox->setText(text.arg(one ? "One file" : QString::number(missed.size()) + " files"));
        mMsgBox->setDetailedText("Missing file" + QString(missed.size()==1 ? "":"s") + ":\n"
                               + QDir::toNativeSeparators(missed.join("\n")));
        QPushButton *ok = mMsgBox->addButton(QMessageBox::Ok);
        mMsgBox->addButton(QMessageBox::Cancel);
        mMsgBox->setDefaultButton(ok);
        if (QTextEdit *edit = mMsgBox->findChild<QTextEdit*>())
            edit->setLineWrapMode(QTextEdit::NoWrap);
        connect(mMsgBox, &QMessageBox::accepted, this, &PathRequest::acceptOpen);
        connect(mMsgBox, &QMessageBox::finished, this, [this]() {
            mMsgBox->deleteLater();
            mMsgBox = nullptr;
            emit done();
        });

        mMsgBox->setModal(true);
        mMsgBox->open();
#ifdef __APPLE__
        mMsgBox->show();
        mMsgBox->raise();
#endif
    }

    return missed.isEmpty();
}

} // namespace path
} // namespace studio
} // namespace gams
