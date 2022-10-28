/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
    setWindowTitle("Select projects base directory");
    mInitialText = ui->laText->text();
    ui->edBaseDir->setMinimumWidth(fontMetrics().height()*30);
    ui->edBaseDir->setToolTip("Base directory: used as base folder to represent the files");
    ui->laBaseDir->setToolTip(ui->edBaseDir->toolTip());
    ui->bDir->setIcon(Theme::icon(":/%1/folder-open-bw"));
    adjustSize();
}

PathRequest::~PathRequest()
{
    delete ui;
}

void PathRequest::init(ProjectRepo *repo, const QString &name, const QString &baseDir, const QVariantList &data)
{
    mData = data;
    mProjectRepo = repo;
    ui->edBaseDir->setText(QDir::toNativeSeparators(baseDir));
    mInitialText = mInitialText.arg("Opening project " + name + ":<br/>%1");
    setWindowTitle("Select base directory for " + name);
}

bool PathRequest::checkProject()
{
    int count;
    int ignored;
    QStringList missed;
    int ok = mProjectRepo->checkRead(mData, count, ignored, missed, baseDir());
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

void PathRequest::on_edBaseDir_textEdited(const QString &text)
{
    updateEditColor(ui->edBaseDir, text);
}

void PathRequest::updateEditColor(QLineEdit *edit, const QString &text)
{
    QDir dir(text.trimmed());
    if (!dir.exists()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        QPalette pal = edit->palette();
        pal.setColor(QPalette::Text, Theme::color(Theme::Mark_errorFg));
        edit->setPalette(pal);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        edit->setPalette(QPalette());
        checkProject();
    }
}

QString PathRequest::baseDir() const
{
    return QDir::fromNativeSeparators(ui->edBaseDir->text()).trimmed();
}

void PathRequest::on_bDir_clicked()
{
    showDirDialog("Select Base Directory", ui->edBaseDir);
}

void PathRequest::showDirDialog(const QString &title, QLineEdit *lineEdit)
{
    QFileDialog *dialog = new QFileDialog(this, title, ui->edBaseDir->text().trimmed());
    dialog->setFileMode(QFileDialog::Directory);
    connect(dialog, &QFileDialog::accepted, this, [lineEdit, dialog]() {
        if (dialog->selectedFiles().count() == 1) {
            QDir dir(dialog->selectedFiles().at(0).trimmed());
            if (dir.exists()) lineEdit->setText(QDir::toNativeSeparators(dir.path()));
        }
    });
    connect(dialog, &QFileDialog::finished, this, [dialog]() { dialog->deleteLater(); });
    dialog->setModal(true);
    dialog->open();
}

} // namespace path
} // namespace studio
} // namespace gams
