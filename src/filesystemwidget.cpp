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
#include "filesystemwidget.h"
#include "ui_filesystemwidget.h"
#include "filesystemmodel.h"
#include "file/filetype.h"
#include "theme.h"
#include "logger.h"

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
    mFilterModel->setFilterCaseSensitivity(FileType::fsCaseSense());
    auto oldModel = ui->directoryView->selectionModel();
    ui->directoryView->setModel(mFilterModel);
    connect(mFileSystemModel, &FileSystemModel::dataChanged, this, &FileSystemWidget::updateButtons);
    ui->directoryView->viewport()->installEventFilter(this);
    delete oldModel;
    connect(mFileSystemModel, &FileSystemModel::selectionCountChanged, this, &FileSystemWidget::selectionCountChanged);
    connect(mFileSystemModel, &FileSystemModel::missingFiles, this, [this](const QStringList &files) {
        mMissingFiles = files;
        selectionCountChanged(mFileSystemModel->selectionCount());
    });
    connect(ui->edFilter, &FilterLineEdit::regExpChanged, this, [this](const QRegularExpression &regExp) {
        mFilterModel->setFilterRegularExpression(regExp);
        QModelIndex rootIndex = mFileSystemModel->index(mWorkingDirectory);
        ui->directoryView->setRootIndex(mFilterModel->mapFromSource(rootIndex));
    });
    mUncommonFiles << "*.log" << "*.log~*" << "*.lxi" << "*.lst" << "*.efi"
                   << "%1_files.txt" << "conf_%1" << "data_%1" << "static_%1" << "renderer_%1";
}

FileSystemWidget::~FileSystemWidget()
{
    delete mDelegate;
    delete mFilterModel;
    delete mFileSystemModel;
    delete ui;
}

void FileSystemWidget::setInfo(const QString &message, bool valid) {
    QString extraText;
    QString toolTip;
    if (!mMissingFiles.isEmpty()) {
        extraText = QString(" (%1 files missing)").arg(mMissingFiles.count());
        toolTip = "Missing files:\n" + mMissingFiles.join("\n");
    }
    auto palette = qApp->palette();
    if (valid) {
        palette.setColor(ui->laInfo->foregroundRole(), Theme::color(Theme::Normal_Green));
        ui->createButton->setText("Save Changes");
    } else {
        palette.setColor(ui->laInfo->foregroundRole(), Theme::color(Theme::Normal_Red));
        ui->createButton->setText("Create");
        ui->createButton->setEnabled(true);
    }
    ui->laInfo->setPalette(palette);
    ui->laInfo->setText(message + extraText);
    ui->laInfo->setToolTip(toolTip);
}

void FileSystemWidget::setModelName(const QString &modelName)
{
    if (modelName.isEmpty()) {
        mFilterModel->setUncommonRegExp(QRegularExpression());
        return;
    }
    QStringList uncommonFiles;
    for (const QString &rawFile: std::as_const(mUncommonFiles)) {
        uncommonFiles << (rawFile.contains("%1") ? rawFile.arg(modelName.toLower()) : rawFile);
    }
    QString pattern = uncommonFiles.join("|").replace('.', "\\.").replace('?', '.').replace("*", ".*");
    pattern = QString("^(%1)$").arg(pattern);
    QRegularExpression rex(pattern);
    if (FileType::fsCaseSense()) rex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    mFilterModel->setUncommonRegExp(rex);
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

int FileSystemWidget::selectionCount()
{
    return mFileSystemModel->selectionCount();
}

void FileSystemWidget::clearMissingFiles()
{
    mMissingFiles.clear();
}

void FileSystemWidget::selectFilter()
{
    ui->edFilter->setFocus();
}

void FileSystemWidget::clearSelection()
{
    mFileSystemModel->clearSelection();
}

void FileSystemWidget::on_createButton_clicked()
{
    emit createClicked();
    mMissingFiles.clear();
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

void FileSystemWidget::on_cbUncommon_clicked(bool checked)
{
    mFilterModel->setHideUncommonFiles(checked);
}

void FileSystemWidget::selectionCountChanged(int count)
{
    ui->laSelectCount->setText(QString(count ? "%1 file%2 selected" : "no file selected")
                               .arg(count).arg(count > 1 ? "s" : ""));
    emit modified();
}

void FileSystemWidget::updateButtons()
{
    ui->createButton->setEnabled(true);
    ui->clearButton->setEnabled(mFileSystemModel->hasSelection());
}

void FileSystemWidget::setupViewModel()
{
    if (mWorkingDirectory.isEmpty()) return;
    mFileSystemModel->setRootPath(mWorkingDirectory);
    QModelIndex rootIndex = mFileSystemModel->index(mWorkingDirectory);
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
