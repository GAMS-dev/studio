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
#include "filereferencewidget.h"
#include "ui_filereferencewidget.h"

#include <QKeyEvent>

namespace gams {
namespace studio {
namespace reference {

FileReferenceWidget::FileReferenceWidget(Reference* ref, ReferenceViewer *parent) :
    ui(new Ui::FileReferenceWidget),
    mFilterActivated(false),
    mReference(ref),
    mReferenceViewer(parent)
{
    ui->setupUi(this);

    mProxyModel = new FileUsedFilterProxyModel(this);
    mFileUsedModel = new FileUsedTreeModel(this);
    mProxyModel->setFilterKeyColumn(-1);
    mProxyModel->setSourceModel( mFileUsedModel );
    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    ui->fileReferenceTreeView->setModel( mProxyModel );
    ui->fileReferenceTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->fileReferenceTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->fileReferenceTreeView->resizeColumnToContents( static_cast<int>(FileReferenceItemColumn::Location) );
    ui->fileReferenceTreeView->resizeColumnToContents( static_cast<int>(FileReferenceItemColumn::Type) );
    ui->fileReferenceTreeView->setColumnHidden( static_cast<int>(FileReferenceItemColumn::GlobalLineNumber), true);
//    ui->fileReferenceTreeView->setColumnHidden( static_cast<int>(FileReferenceItemColumn::Id), true);
    ui->fileReferenceTreeView->setColumnHidden( static_cast<int>(FileReferenceItemColumn::ParentId), true);
    ui->fileReferenceTreeView->setColumnHidden( static_cast<int>(FileReferenceItemColumn::FirstEntry), true);
    ui->fileReferenceTreeView->expandAll();

    ui->showFileOnceCheckBox->setCheckState(Qt::Checked);

    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(ui->fileReferenceTreeView);
    setTabOrder(ui->showFileOnceCheckBox, ui->fileReferenceTreeView);

    connect(ui->fileReferenceTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileReferenceWidget::linkToReferenceItem);
    connect(ui->fileReferenceTreeView, &QAbstractItemView::doubleClicked, this, &FileReferenceWidget::jumpToReferenceItem);

    connect( mFileUsedModel, &FileUsedTreeModel::modelReset, this, &FileReferenceWidget::expandResetModel);

    connect(ui->showFileOnceCheckBox, static_cast<void(QCheckBox::*)(Qt::CheckState)>(&QCheckBox::checkStateChanged), this, [=](Qt::CheckState state) {
        mProxyModel->showFileOnceChanged(state==Qt::Checked);
        mFileUsedModel->resetModel();
        emit mReference->reloadFiledUsedTabFinished(state==Qt::Checked);
    });

}

FileReferenceWidget::~FileReferenceWidget()
{
    delete ui;
    if (mProxyModel)
        delete mProxyModel;
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

bool FileReferenceWidget::isViewCompact()
{
    return (ui->showFileOnceCheckBox->checkState()==Qt::Checked);
}

void FileReferenceWidget::activateFilter()
{
    // not the first time or model not loaded
    if (mFilterActivated || !mReference)
        return;

    mFilterActivated = true;
    mReference->filterFileUsedReference();
    emit mReference->reloadFiledUsedTabFinished(ui->showFileOnceCheckBox->checkState()==Qt::Checked);
}

void FileReferenceWidget::deActivateFilter()
{
    mFilterActivated = false;
}

void FileReferenceWidget::keyPressEvent(QKeyEvent *e)
{
    QModelIndex currentIndex = ui->fileReferenceTreeView->currentIndex();
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        if (!ui->fileReferenceTreeView->selectionModel()->hasSelection()) {
            ui->fileReferenceTreeView->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        } else if (currentIndex.isValid()) {
            jumpToReferenceItem(currentIndex);
        }
        e->accept();
    } else if (e->key() == Qt::Key_Escape) {
        if (ui->fileReferenceTreeView->hasFocus()) {
            ui->fileReferenceTreeView->selectionModel()->clearSelection();
            ui->fileReferenceTreeView->clearFocus( );
            ui->showFileOnceCheckBox->setFocus();
        } else if (ui->showFileOnceCheckBox->hasFocus()) {
            ui->showFileOnceCheckBox->clearFocus( );
            parentWidget()->setFocus();
        } else {
            parentWidget()->setFocus();
        }
        e->accept();
    } else if (e->key() == Qt::Key_Tab) {
        if (ui->fileReferenceTreeView->hasFocus()) {
            ui->fileReferenceTreeView->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        } else {
            ui->fileReferenceTreeView->clearFocus( );
        }
        e->accept();
    }
    QWidget::keyPressEvent(e);
}

void FileReferenceWidget::resetModel()
{
    mFileUsedModel->resetModel();
}

void FileReferenceWidget::initModel()
{
    mFileUsedModel->initModel(mReference);
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
    ReferenceItem item(-1, "", ReferenceDataType::Unknown, location.toString(), 0, 0);
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
        ReferenceItem item(-1, "", ReferenceDataType::Unknown, location.toString(), line.toInt(), 0);
        emit mReferenceViewer->jumpTo( item );
    } else {
        jumpToFile(index);
    }

}

void FileReferenceWidget::linkToReferenceItem(const QItemSelection &selected, const QItemSelection &deselected) //const QModelIndex &index)
{
    Q_UNUSED(deselected)
    if (selected.indexes().size() > 0) {
        QModelIndex idx = /*mProxyModel->mapToSource( */selected.indexes().first() ;// );
        int id = idx.siblingAtColumn(static_cast<int> (FileReferenceItemColumn::Id)).data().toInt();
        emit mReferenceViewer->referenceTo( referenceItem(id) );
    }
}

void FileReferenceWidget::selectFileReference(int id, bool compactView)
{
    ui->showFileOnceCheckBox->setCheckState( compactView ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    QModelIndexList typeindices = mFileUsedModel->match(mFileUsedModel->index(0, static_cast<int>(FileReferenceItemColumn::Id)),
                                                             Qt::DisplayRole,
                                                             id, -1, Qt::MatchExactly|Qt::MatchRecursive);
    if (typeindices.size() <= 0) {
        return;
    }

    for(const QModelIndex &idx: std::as_const(typeindices)) {
        if (idx.isValid()) {
            QModelIndex srcidx = mProxyModel->mapFromSource(idx);
            ui->fileReferenceTreeView->setCurrentIndex(srcidx);
            QItemSelection selection(srcidx, srcidx.siblingAtColumn(static_cast<int>(FileReferenceItemColumn::FirstEntry)));
            ui->fileReferenceTreeView->selectionModel()->select(selection,  QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
            break;
        }
    }
}

int FileReferenceWidget::currentReferenceId()
{
    QModelIndex  index =  ui->fileReferenceTreeView->currentIndex();
    if (index.isValid())
        return (ui->fileReferenceTreeView->model()->data(index.siblingAtColumn(static_cast<int>(FileReferenceItemColumn::Id))).toInt() );

    return -1;
}

ReferenceItem FileReferenceWidget::referenceItem(int id)
{
    ReferenceItem item(-1, "", ReferenceDataType::Unknown, "", -1, -1);

    QModelIndexList typeindices = mFileUsedModel->match(mFileUsedModel->index(0, static_cast<int>(FileReferenceItemColumn::Id)),
                                                        Qt::DisplayRole,
                                                        id, -1, Qt::MatchExactly|Qt::MatchRecursive);
    if (typeindices.size() <= 0) {
        return item;
    }

    if (typeindices.first().isValid()) {
        QModelIndex idx = mProxyModel->mapFromSource( typeindices.first()  );
        if (typeindices.first().parent().isValid()) {
            QVariant location( ui->fileReferenceTreeView->model()->data(idx.parent().siblingAtColumn((int)FileReferenceItemColumn::Location)).toString() );
            QVariant line    ( ui->fileReferenceTreeView->model()->data(idx.siblingAtColumn((int)FileReferenceItemColumn::LocalLineNumber)).toInt() );
            item.location = location.toString();
            item.lineNumber = line.toInt();
       } else {
            QVariant location( ui->fileReferenceTreeView->model()->data(idx.siblingAtColumn((int)FileReferenceItemColumn::Location)).toString() );
            QVariant line    ( ui->fileReferenceTreeView->model()->data(idx.siblingAtColumn((int)FileReferenceItemColumn::LocalLineNumber)).toInt() );
            item.location = location.toString();
            item.lineNumber = line.toInt();
       }
    }
    return item;
}

} // namespace reference
} // namespace studio
} // namespace gams


