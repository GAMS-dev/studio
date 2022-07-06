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
#include "filesystemwidget.h"
#include "ui_filesystemwidget.h"
#include "filesystemmodel.h"
#include "theme.h"

#include <QMessageBox>
#include <QMouseEvent>

namespace gams {
namespace studio {
namespace fs {

const int CanDownloadRole = Qt::UserRole + 5;

FileSystemWidget::FileSystemWidget(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::FileSystemWidget)
    , mFileSystemModel(new FileSystemModel(this))
    , mFilterModel(new FilteredFileSystemModel(this))
{
    ui->setupUi(this);
    mDelegate = new FileSystemItemDelegate(this);
    mFilterModel->setSourceModel(mFileSystemModel);
    auto oldModel = ui->directoryView->selectionModel();
    ui->directoryView->setModel(mFilterModel);
    connect(mFileSystemModel, &FileSystemModel::dataChanged, this, &FileSystemWidget::updateButtons);
    ui->directoryView->viewport()->installEventFilter(this);
    delete oldModel;
    setShowProtection(true);
}

void FileSystemWidget::clear()
{
    mFileSystemModel->clearSelection();
}

QString FileSystemWidget::assemblyFileName() const {
    return mModelAssemblyFile;
}

void FileSystemWidget::setAssemblyFileName(const QString &file) {
    mModelAssemblyFile = file;
    QFileInfo fi(mModelAssemblyFile);
    mValidAssemblyFile = fi.exists();
    if (fi.exists()) {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Green));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText("File " + fi.fileName() + " found.");
        ui->createButton->setText("Save Changes");
    } else {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Red));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText("No file " + fi.fileName() + " found!");
        ui->createButton->setText("Create");
    }
}

bool FileSystemWidget::validAssemblyFile() const
{
    return mValidAssemblyFile;
}

QStringList FileSystemWidget::selectedFiles()
{
    if (mFileSystemModel)
        return mFileSystemModel->selectedFiles();
    return QStringList();
}

void FileSystemWidget::setSelectedFiles(const QStringList &files)
{
    mFileSystemModel->setSelectedFiles(files);
    ui->createButton->setEnabled(false);
}

void FileSystemWidget::setWorkingDirectory(const QString &workingDirectory)
{
    mWorkingDirectory = workingDirectory;
    setupViewModel();
}

QString FileSystemWidget::workingDirectory() const
{
    return mWorkingDirectory;
}

bool FileSystemWidget::showProtection() const
{
    return mShowProtection;
}

void FileSystemWidget::setShowProtection(bool showProtection)
{
    mShowProtection = showProtection;
    ui->directoryView->setItemDelegate(showProtection ? mDelegate : nullptr);
}

bool FileSystemWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        QModelIndex ind = ui->directoryView->indexAt(me->pos());
        if (me->button() == Qt::LeftButton && showProtection()) {
            QRect rect = ui->directoryView->visualRect(ind);
            if (rect.isValid() && me->pos().x() > rect.right() - rect.height()) {
                mFileSystemModel->setData(ind, !mFileSystemModel->data(ind).toBool(), CanDownloadRole);
                return true;
            }
        }
    }
    return false;
}

void FileSystemWidget::on_createButton_clicked()
{
    emit createButtonClicked();
    ui->createButton->setEnabled(false);
}

void FileSystemWidget::on_selectAllButton_clicked()
{
    mFileSystemModel->selectAll();
}

void FileSystemWidget::on_clearButton_clicked()
{
    mFileSystemModel->clearSelection();
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

void FileSystemWidget::updateButtons()
{
    ui->createButton->setEnabled(true);
    ui->clearButton->setEnabled(!selectedFiles().isEmpty());
}

void FileSystemWidget::setupViewModel()
{
    if (mWorkingDirectory.isEmpty())
        return;

    mFileSystemModel->setRootPath(mWorkingDirectory);
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

FileSystemItemDelegate::FileSystemItemDelegate(QObject *parent): QStyledItemDelegate(parent)
{}

void FileSystemItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return;
    QStyleOptionViewItem opt(option);
    opt.state.setFlag(QStyle::State_Selected, false);
    opt.textElideMode = Qt::ElideMiddle;
    opt.palette.setColor(QPalette::Highlight, Qt::transparent);
    bool isFile = true;
    QRect btRect = opt.rect;
    if (isFile) {
        btRect.setLeft(opt.rect.right() - opt.rect.height());
        btRect = btRect.marginsRemoved(QMargins(2,2,2,2));
        opt.rect.setRight(opt.rect.right() - opt.rect.height());
    }
    QStyledItemDelegate::paint(painter, opt, index);
    if (isFile) {
        bool act = false;
        if (index.row() == 4)
            act = false;
        if (opt.state.testFlag(QStyle::State_MouseOver))
            act = true;
        QIcon icon = index.model()->data(index, CanDownloadRole).toBool() ? Theme::icon(":/%1/triangle-down")
                                                                          : Theme::icon(":/%1/triangle-left");
        Theme::icon(":/%1/triangle-down").paint(painter, btRect, Qt::AlignCenter, act ? QIcon::Normal : QIcon::Disabled);
    }
}

}
}
}
