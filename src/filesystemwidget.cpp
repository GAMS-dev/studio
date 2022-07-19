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
    connect(mFileSystemModel, &FileSystemModel::selectionCountChanged, this, &FileSystemWidget::selectionCountChanged);
}

void FileSystemWidget::setInfo(const QString &message, bool valid) {
    if (valid) {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Green));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText(message);
        ui->createButton->setText("Save Changes");
    } else {
        auto palette = ui->assemblyFileLabel->palette();
        palette.setColor(ui->assemblyFileLabel->foregroundRole(),
                         Theme::color(Theme::Normal_Red));
        ui->assemblyFileLabel->setPalette(palette);
        ui->assemblyFileLabel->setText(message);
        ui->createButton->setText("Create");
    }
}

QStringList FileSystemWidget::selectedFiles()
{
    if (mFileSystemModel)
        return mFileSystemModel->selectedFiles(mShowProtection);
    return QStringList();
}

void FileSystemWidget::setSelectedFiles(const QStringList &files)
{
    mFileSystemModel->setSelectedFiles(files);
    ui->createButton->setEnabled(false);
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

QString FileSystemWidget::workingDirectory() const
{
    return mWorkingDirectory;
}

void FileSystemWidget::setWorkingDirectory(const QString &workingDirectory)
{
    mWorkingDirectory = workingDirectory;
    setupViewModel();
}

void FileSystemWidget::setShowProtection(bool showProtection)
{
    mShowProtection = showProtection;
    ui->createButton->setText(showProtection ? "Save" : "Create");
    ui->directoryView->setItemDelegate(showProtection ? mDelegate : nullptr);
}

void FileSystemWidget::setCreateVisible(bool visible)
{
    ui->createButton->setVisible(visible);
}

void FileSystemWidget::clearSelection()
{
    mFileSystemModel->clearSelection();
}

void FileSystemWidget::on_createButton_clicked()
{
    emit createClicked();
    ui->createButton->setEnabled(false);
}

void FileSystemWidget::on_selectAllButton_clicked()
{
    mFileSystemModel->selectAll();
}

void FileSystemWidget::on_clearButton_clicked()
{
    mFileSystemModel->clearSelection();
}

void FileSystemWidget::updateButtons()
{
    ui->createButton->setEnabled(true);
    ui->clearButton->setEnabled(mFileSystemModel->hasSelection());
}

void FileSystemWidget::setupViewModel()
{
    if (mWorkingDirectory.isEmpty())
        return;

    mFileSystemModel->setRootPath(mWorkingDirectory);
    auto rootIndex = mFileSystemModel->index(mWorkingDirectory);
    ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
}

bool FileSystemWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::ToolTip) {
        QPoint pos = ui->directoryView->viewport()->mapFromGlobal(QCursor::pos());
        QModelIndex ind = ui->directoryView->indexAt(pos);
        QRect rect = ui->directoryView->visualRect(ind);
        QModelIndex sInd = mFilterModel->mapToSource(ind);
        bool invisible = (mFileSystemModel->data(sInd, Qt::CheckStateRole).toInt() != Qt::Checked);
        if (!rect.isValid() || pos.x() <= rect.right() - rect.height() || invisible) {
            setToolTip("");
        } else if (mFileSystemModel->data(sInd, WriteBackRole).toBool())
            setToolTip("<p style='white-space:pre'><b>Write back</b> the file from server replacing the local file</p>");
        else
            setToolTip("<p style='white-space:pre'><b>Only send</b> the file to the server keeping the local file unchanged</p>");

    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        QModelIndex ind = ui->directoryView->indexAt(me->pos());
        QRect rect = ui->directoryView->visualRect(ind);
        if (me->button() == Qt::LeftButton && showProtection()) {
            if (rect.isValid() && me->pos().x() > rect.right() - rect.height()) {
                QModelIndex sInd = mFilterModel->mapToSource(ind);
                if (mFileSystemModel->data(sInd, Qt::CheckStateRole).toInt() == Qt::Checked)
                    mFileSystemModel->setData(sInd, !mFileSystemModel->data(sInd, WriteBackRole).toBool(), WriteBackRole);
                return true;
            }
        }
    }
    return false;
}

void FileSystemItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return;
    const FilteredFileSystemModel *model = static_cast<const FilteredFileSystemModel*>(index.model());
    QStyleOptionViewItem opt(option);
    opt.state.setFlag(QStyle::State_Selected, false);
    opt.textElideMode = Qt::ElideMiddle;
    opt.palette.setColor(QPalette::Highlight, Qt::transparent);
    bool isFile = (model->data(index, Qt::CheckStateRole).toInt() == Qt::Checked) && !model->isDir(index);
    QRect btRect = opt.rect;
    if (isFile) {
        btRect.setLeft(opt.rect.right() - opt.rect.height());
        btRect = btRect.marginsRemoved(QMargins(2,2,2,2));
        opt.rect.setRight(opt.rect.right() - opt.rect.height());
    }
    QStyledItemDelegate::paint(painter, opt, index);
    if (isFile) {
        bool writeBack = model->data(index, WriteBackRole).toBool();
        QIcon icon = writeBack ? Theme::icon(":/solid/out-in") : Theme::icon(":/solid/out");
        icon.paint(painter, btRect, Qt::AlignCenter, writeBack ? QIcon::Normal : QIcon::Disabled);
    }
}

}
}
}
