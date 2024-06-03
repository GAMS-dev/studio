/*
 * This file is part of the GAMS Studio project.
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
#include "filereferencewidget.h"
#include "ui_filereferencewidget.h"

namespace gams {
namespace studio {
namespace reference {

FileReferenceWidget::FileReferenceWidget(Reference* ref, ReferenceViewer *parent) :
    ui(new Ui::FileReferenceWidget),
    mReference(ref),
    mReferenceViewer(parent)
{
    ui->setupUi(this);
    mFileUsedModel = new FileUsedTreeModel(this);
    ui->fileReferenceTreeView->setModel( mFileUsedModel );

    ui->fileReferenceTreeView->expandAll();

    ui->fileReferenceTreeView->resizeColumnToContents( (int)FileReferenceItemColumn::Location );
    ui->fileReferenceTreeView->resizeColumnToContents( (int)FileReferenceItemColumn::Type );
    ui->fileReferenceTreeView->setColumnHidden( (int)FileReferenceItemColumn::GlobalLineNumber, true );
    ui->fileReferenceTreeView->setColumnHidden( (int)FileReferenceItemColumn::Id, true);

    connect(ui->fileReferenceTreeView, &QAbstractItemView::doubleClicked, this, &FileReferenceWidget::jumpToReferenceItem);
    connect( mFileUsedModel, &FileUsedTreeModel::modelReset, this, &FileReferenceWidget::expandResetModel);
}

FileReferenceWidget::~FileReferenceWidget()
{
    delete ui;
    if (mFileUsedModel)
        delete mFileUsedModel;
}

QList<QHeaderView *> FileReferenceWidget::headers()
{
    return QList<QHeaderView *>() << ui->fileReferenceTreeView->header();
}

bool FileReferenceWidget::isModelLoaded() const
{
    return mFileUsedModel->isModelLoaded();
}

void FileReferenceWidget::resetModel()
{
    mFileUsedModel->resetModel();
}

void FileReferenceWidget::initModel()
{
    if (!mFileUsedModel->isModelLoaded()) {
        mFileUsedModel->initModel(mReference);
    }

    mFileUsedModel->resetModel();
}

void FileReferenceWidget::initModel(Reference *ref)
{
    if (!ref)
       return;

    mReference = ref;
    initModel();
}

void FileReferenceWidget::expandResetModel()
{
    ui->fileReferenceTreeView->expandAll();
    ui->fileReferenceTreeView->resizeColumnToContents( (int)FileReferenceItemColumn::Location );
    ui->fileReferenceTreeView->resizeColumnToContents( (int)FileReferenceItemColumn::Type );
}

void FileReferenceWidget::jumpToFile(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QVariant location( ui->fileReferenceTreeView->model()->data(index.siblingAtColumn((int)FileReferenceItemColumn::Location)).toString() );
    ReferenceItem item(-1, ReferenceDataType::Unknown, location.toString(), 0, 0);
    emit mReferenceViewer->jumpTo( item );
}

void FileReferenceWidget::jumpToReferenceItem(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QModelIndex  parentIndex =  ui->fileReferenceTreeView->model()->parent(index);
    if (parentIndex.isValid()) {
        QVariant location( ui->fileReferenceTreeView->model()->data(parentIndex.siblingAtColumn((int)FileReferenceItemColumn::Location)).toString() );
        QVariant line    ( ui->fileReferenceTreeView->model()->data(index.siblingAtColumn((int)FileReferenceItemColumn::LocalLineNumber)).toInt() );
        ReferenceItem item(-1, ReferenceDataType::Unknown, location.toString(), line.toInt(), 0);
        emit mReferenceViewer->jumpTo( item );
    } else {
        jumpToFile(index);
    }

}

} // namespace reference
} // namespace studio
} // namespace gams
