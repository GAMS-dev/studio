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
#include "ui_pathrequest.h"
#include "projectrepo.h"
#include <QFileDialog>
#include <QPushButton>

namespace gams {
namespace studio {
namespace path {

PathRequest::PathRequest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PathRequest)
{
    ui->setupUi(this);
    setModal(true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowTitle("Missing file(s) for project ");
    mInitialText = ui->laText->text();
    adjustSize();
}

PathRequest::~PathRequest()
{
    delete ui;
}

void PathRequest::init(ProjectRepo *repo, const QString &name, const QString &baseDir, const QVariantMap &data)
{
    mData = data;
    mProjectRepo = repo;
    mBaseDir = QDir::toNativeSeparators(baseDir);
    mInitialText = mInitialText.arg("Opening project " + name + ":<br/>%1");
    setWindowTitle("Missing file(s) for project " + name);
}

bool PathRequest::checkProject()
{
    int count;
    int ignored;
    QStringList missed;
    int ok = mProjectRepo->checkRead(mData, count, ignored, missed, mBaseDir);
    if (ok) {
        if (ignored)
            ui->laText->setText(QString("All referenced necessary files found, %1 ignored.")
                                .arg(QString::number(ignored) + (ignored==1 ? " file" : "files")));
        else
            ui->laText->setText(QString("All referenced files found."));
        ui->laText->setToolTip(QString());
    } else {
        bool one = (missed.size() == 1);
        ui->laText->setText(mInitialText.arg(one ? "One file" : QString::number(missed.size()) + " files"));
        ui->laText->setToolTip("Missing file"+QString(missed.size()==1 ? "":"s")+":\n" + missed.join("\n"));
    }
    return missed.isEmpty();
}


} // namespace path
} // namespace studio
} // namespace gams
