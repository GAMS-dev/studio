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
#include <QFileInfo>
#include <QClipboard>

#include "option/newoption/solveroptiontablemodel.h"
#include "option/newoption/solveroptioneditor.h"
#include "option/solveroptiondefinitionmodel.h"
#include "file/filetype.h"

#include "exception.h"
#include "msgbox.h"
#include "settings.h"
#include "ui_optionwidget.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

SolverOptionEditor::SolverOptionEditor(const QString &solverName,
                                       const QString &optionFilePath,
                                       const QString &optDefFileName,
                                       const FileKind &kind,
                                       const FileId &id,
                                       const QString &encodingName,
                                       QWidget *parent) :
    OptionWidget(true, parent),
    mSolverName(solverName),
    mEncoding(encodingName.isEmpty() ? "UTF-8" : encodingName),
    mLocation(optionFilePath),
    mDefinitionFileName(optDefFileName),
    mFileKind(kind),
    mFileId(id),
    mModified(false),
    mFileHasChangedExtern(false)
{
    mOptionTokenizer = new OptionTokenizer(mDefinitionFileName);
    if (!mOptionTokenizer->getOption()->available())
        EXCEPT() << "Could not find or load OPT library for opening '" << mLocation << "'. Please check your GAMS installation.";

    const QList<SolverOptionItem *> optionItem = mOptionTokenizer->readOptionFile(optionFilePath, encodingName);
    mOptionModel = new SolverOptionTableModel(optionItem, mOptionTokenizer,  this);

    mOptionCompleter = new OptionItemDelegate(optionTokenizer(), ui->optionTableView);
    mDefinitionModel = new SolverOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);

    initToolBar();
    initActions();
    initTableView();
    initTreeView();
    initTabNavigation( mIsFileEditor );
    initMessageControl( mIsFileEditor );

    if (!mOptionTokenizer->getOption()->available())  {
        ui->definitionSearch->setReadOnly(true);
        ui->compactViewCheckBox->setEnabled(false);

        const QString msg1 = QString("Unable to open %1 Properly").arg(mLocation);
        const QString msg2 = QString("'%1' is not a valid solver name").arg(mSolverName);
        mOptionTokenizer->logger()->append(QString("%1. %2").arg(msg1, msg2), LogMsgType::Error);
        mOptionTokenizer->logger()->append(QString("An operation on the file contents might not be saved. Try 'Save As' or 'Open As Text' instead."), LogMsgType::Warning);
//        return false;
    }
    else {
        connect(ui->optionTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &SolverOptionEditor::on_selectAndToggleRow, Qt::UniqueConnection);
        connect(ui->optionTableView, &QTableView::customContextMenuRequested, this, &SolverOptionEditor::showOptionContextMenu, Qt::UniqueConnection);
        connect(mOptionModel, &SolverOptionTableModel::newTableRowDropped, this, &SolverOptionEditor::on_newTableRowDropped, Qt::UniqueConnection);

        connect(ui->definitionSearch, &FilterLineEdit::regExpChanged, this, [this]() {
            mDefinitionProxymodel->setFilterRegularExpression(ui->definitionSearch->regExp());
            selectSearchField();
        });

        connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
        connect(ui->definitionTreeView, &QAbstractItemView::doubleClicked, this, &SolverOptionEditor::addOptionFromDefinition);
        connect(ui->definitionTreeView, &QTreeView::customContextMenuRequested, this, &SolverOptionEditor::showDefinitionContextMenu, Qt::UniqueConnection);

        connect(ui->definitionGroup, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
            mDefinitionModel->loadOptionFromGroup( mDefinitionGroupModel->data(mDefinitionGroupModel->index(index, 1)).toInt() );
            mOptionModel->on_groupDefinitionReloaded();
        });

        connect(this, &SolverOptionEditor::modificationChanged, this, &SolverOptionEditor::setModified, Qt::UniqueConnection);

        connect(mOptionModel, &QAbstractTableModel::dataChanged, this, &SolverOptionEditor::on_dataItemChanged, Qt::UniqueConnection);
        connect(mOptionModel, &QAbstractTableModel::dataChanged, mOptionModel, &SolverOptionTableModel::on_updateOptionItem, Qt::UniqueConnection);
        connect(mOptionModel, &SolverOptionTableModel::solverOptionItemModelChanged, mOptionModel, &SolverOptionTableModel::updateRecurrentStatus, Qt::UniqueConnection);
        connect(mOptionModel, &SolverOptionTableModel::solverOptionModelChanged, mDefinitionModel, &SolverOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);
        connect(mOptionModel, &SolverOptionTableModel::solverOptionItemModelChanged, mDefinitionModel, &SolverOptionDefinitionModel::modifyOptionDefinitionItem, Qt::UniqueConnection);
        connect(mOptionModel, &OptionTableModel::optionItemRemoved, mOptionModel, &SolverOptionTableModel::on_removeOptionItem, Qt::UniqueConnection);

        connect( mOptionCompleter, &OptionCompleterDelegate::closeEditor, this, &SolverOptionEditor::completeEditingOption, Qt::UniqueConnection );

        connect(this, &SolverOptionEditor::compactViewChanged, mDefinitionModel, &SolverOptionDefinitionModel::on_compactViewChanged, Qt::UniqueConnection);

        mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
//        return true;
    }

}

SolverOptionEditor::~SolverOptionEditor()
{
    if (mOptionTokenizer)
        delete mOptionTokenizer;
    if (mOptionCompleter)
        delete mOptionCompleter;
    if (mDefinitionGroupModel)
        delete mDefinitionGroupModel;
    if (mDefinitionModel)
        delete mDefinitionModel;
    if (mDefinitionGroupModel)
        delete mDefinitionProxymodel;
    if (mOptionModel)
        delete mOptionModel;
}

bool SolverOptionEditor::saveAs(const QString &location)
{
    setModified(false);

    const bool success = mOptionTokenizer->writeOptionFile(mOptionModel->getCurrentListOfOptionItems(), location, mEncoding);
    if (mLocation != location) {
        bool warning = false;
        if (FileType::from(FileKind::Opt) == FileType::from(QFileInfo(location).fileName()) &&
            QString::compare(QFileInfo(mLocation).completeBaseName(), QFileInfo(location).completeBaseName(), Qt::CaseInsensitive) != 0) {
            mOptionTokenizer->logger()->append(QString("Solver option file name '%1' has been changed. Saved options into '%2' may cause solver option editor to display the contents improperly.")
                                                   .arg(QFileInfo(mLocation).completeBaseName(), QFileInfo(location).fileName())
                                                   , LogMsgType::Warning);
            warning = true;
        }
        if (FileType::from(FileKind::Opt) != FileType::from(QFileInfo(location).fileName()) &&
            FileType::from(FileKind::Pf) != FileType::from(QFileInfo(location).fileName())) {
            mOptionTokenizer->logger()->append(QString("Unrecognized file suffix '%1'. Saved options into '%2' may cause this editor to display the contents improperly.")
                                                   .arg(QFileInfo(location).fileName(), QFileInfo(location).fileName())
                                                   , LogMsgType::Warning);
            warning = true;
        }
        if (!warning)
            mOptionTokenizer->logger()->append(QString("Saved options into %1").arg(location), LogMsgType::Info);
        mLocation = location;
    } else {
        mOptionTokenizer->logger()->append(QString("Saved options into %1").arg(mLocation), LogMsgType::Info);
    }
    return success;

}


QString SolverOptionEditor::getSolverName() const
{
    return mSolverName;
}

void SolverOptionEditor::toggleCommentOption()
{
    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if ( isThereARow() && !isThereARowSelection() && !isEverySelectionARow() )
        return;

    bool modified = false;
    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    for(int i=0; i<selection.count(); ++i) {
        on_toggleRowHeader( selection.at(i).row() );
        modified = true;
    }
    for(int i=0; i<selection.count(); ++i) {
        on_selectRow( selection.at(i).row() );
    }

    if (!indexSelection.isEmpty())
        ui->optionTableView->selectionModel()->setCurrentIndex( indexSelection.first(), QItemSelectionModel::Current );
    ui->optionTableView->setFocus();

    if (modified) {
        setModified(modified);
    }
}

void SolverOptionEditor::on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    setModified(true);

    QModelIndexList toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                                 Qt::DisplayRole,
                                                                                 mOptionModel->data( topLeft, Qt::DisplayRole), 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                     Qt::DisplayRole,
                                                                     mOptionModel->data( topLeft, Qt::DisplayRole), 1);
    }

    for(const QModelIndex &item : std::as_const(toDefinitionItems)) {
        if (Qt::CheckState(mOptionModel->headerData(item.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
            continue;
        ui->definitionTreeView->selectionModel()->select(
            QItemSelection (
                ui->definitionTreeView->model ()->index (item.row() , 0),
                ui->definitionTreeView->model ()->index (item.row(), ui->definitionTreeView->model ()->columnCount () - 1)),
            QItemSelectionModel::Select);
        ui->definitionTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    ui->optionTableView->selectionModel()->select(topLeft, QItemSelectionModel::Select);
    updateActionsState();
}

void SolverOptionEditor::on_newTableRowDropped(const QModelIndex &index)
{
    disconnect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionEditor::findAndSelectionOptionFromDefinition);
    updateTableColumnSpan();
    ui->optionTableView->selectRow(index.row());
    const QString optionName = mOptionModel->data(index.siblingAtColumn(mOptionModel->column_key()), Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->definitionTreeView->model()->match(ui->definitionTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                               Qt::DisplayRole,
                                                                               optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(const QModelIndex &item : std::as_const(definitionItems)) {
        ui->definitionTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }
    emit itemCountChanged(mOptionModel->rowCount());

    if (mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumStr &&
        mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumInt &&
        mOptionTokenizer->getOption()->getOptionSubType(optionName) != optsubNoValue)
        ui->optionTableView->edit( mOptionModel->index(index.row(), mOptionModel->column_value() ));

    showOptionDefinition(true);
    connect(ui->definitionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SolverOptionEditor::findAndSelectionOptionFromDefinition, Qt::UniqueConnection);
    updateActionsState();
}

bool SolverOptionEditor::saveOptionFile(const QString &location)
{
    return saveAs(location);
}

void SolverOptionEditor::on_reloadSolverOptionFile(const QString &encodingName)
{
    if (mEncoding != encodingName) {
        mEncoding = encodingName.isEmpty() ? "UTF-8" : encodingName;
        mOptionTokenizer->logger()->append(QString("Loading options from %1 with %2 encoding")
                                               .arg(mLocation, encodingName), LogMsgType::Info);
    } else if (mFileHasChangedExtern)
        mOptionTokenizer->logger()->append(QString("Loading options from %1").arg(mLocation), LogMsgType::Info);
    else
        return;
    mOptionModel->reloadSolverOptionModel( mOptionTokenizer->readOptionFile(mLocation, mEncoding) );
    mFileHasChangedExtern = false;
    setModified(false);
}

void SolverOptionEditor::on_selectAndToggleRow(int logicalIndex)
{
    on_selectRow(logicalIndex);
    on_toggleRowHeader(logicalIndex);
}

void SolverOptionEditor::on_toggleRowHeader(int logicalIndex)
{
    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }
    mOptionModel->on_toggleRowHeader(logicalIndex);
    updateTableColumnSpan();
    setModified(true);

    if (ui->compactViewCheckBox->isChecked())
        on_compactViewCheckBox_stateChanged(Qt::Checked);
}

void SolverOptionEditor::on_compactViewCheckBox_stateChanged(int checkState)
{
    const bool isViewCompact = (Qt::CheckState(checkState) == Qt::Checked);
    refreshOptionTableModel(isViewCompact);
    if (isViewCompact) {
        mOptionTokenizer->logger()->append(QString("activated Compact View, comments are hidden and actions related to comments are either not visible or forbidden"), LogMsgType::Info);
    } else {
        mOptionTokenizer->logger()->append(QString("deactivated Compact View, comments are now visible and all actions are allowed"), LogMsgType::Info);
    }
    emit compactViewChanged(isViewCompact);
}

void SolverOptionEditor::on_messageViewCheckBox_stateChanged(int checkState)
{
    if (Qt::CheckState(checkState) == Qt::Checked)
        ui->optionVSplitter->setSizes(QList<int>({75, 25}));
    else
        ui->optionVSplitter->setSizes(QList<int>({100, 0}));
    ui->messageTabWidget->setVisible(Qt::CheckState(checkState) == Qt::Checked);
}

void SolverOptionEditor::on_openAsTextButton_clicked(bool checked)
{
    Q_UNUSED(checked)
    if (isModified()) {
        int answer = MsgBox::warning("File has been modified", QDir::toNativeSeparators(mLocation)+" has been modified. "+
                                                                   "Saving file before reopening prevents data from being lost.\n\n"+
                                                                   "What do you want to do with the modified file?",
                                     this, "Discard and Open As Text", "Save and Open As Text", "Cancel", 1, 2);
        if (answer == 2)
            return;
        else if (answer == 1)
            saveAs(mLocation);
    }

    MainWindow* main = getMainWindow();
    if (!main) return;

    emit main->projectRepo()->closeFileEditors(fileId());

    FileMeta* fileMeta = main->fileRepo()->fileMeta(fileId());
    PExFileNode* fileNode = main->projectRepo()->findFileNode(this);
    PExProjectNode* project = (fileNode ? fileNode->assignedProject() : nullptr);

    emit main->projectRepo()->openFile(fileMeta, true, project, "", true);
}

void SolverOptionEditor::insertOption()
{
    if (isViewCompact())
        return;

    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        if (mOptionCompleter->currentEditedIndex().isValid() && mOptionCompleter->currentEditedIndex().row()==index.row())
            return;
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    disconnect(mOptionModel, &QAbstractTableModel::dataChanged, mOptionModel, &SolverOptionTableModel::on_updateOptionItem);
    int rowToBeInserted = -1;
    if (isThereARowSelection()) {
        QList<int> rows;
        const auto indexes = ui->optionTableView->selectionModel()->selectedRows();
        for(const QModelIndex &idx : indexes) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        mOptionModel->insertRows(rowToBeInserted, 1, QModelIndex());
        const QModelIndex insertKeyIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_key());
        const QModelIndex insertValueIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_value());
        const QModelIndex insertNumberIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_id());

        mOptionModel->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                          Qt::CheckState(Qt::Checked),
                                                          Qt::CheckStateRole );

        mOptionModel->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
        mOptionModel->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
        if (mOptionModel->column_id() > mOptionModel->column_eol_comment()) {
            const QModelIndex eolCommentIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_eol_comment());
            mOptionModel->setData( eolCommentIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        }
        ui->optionTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
        mOptionModel->setData( insertNumberIndex, -1, Qt::EditRole);
    } else {
        mOptionModel->insertRows(mOptionModel->rowCount(), 1, QModelIndex());
        rowToBeInserted = mOptionModel->rowCount()-1;
        const QModelIndex insertKeyIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_key());
        const QModelIndex insertValueIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_value());
        const QModelIndex insertNumberIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_id());

        mOptionModel->setHeaderData(mOptionModel->rowCount()-1, Qt::Vertical,
                                                          Qt::CheckState(Qt::Checked),
                                                          Qt::CheckStateRole );

        mOptionModel->setData( insertKeyIndex, OptionTokenizer::keyGeneratedStr, Qt::EditRole);
        mOptionModel->setData( insertValueIndex, OptionTokenizer::valueGeneratedStr, Qt::EditRole);
        if (mOptionModel->column_id() > mOptionModel->column_eol_comment()) {
            QModelIndex eolCommentIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_eol_comment());
            mOptionModel->setData( eolCommentIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        }
        ui->optionTableView->scrollTo(insertKeyIndex, QAbstractItemView::EnsureVisible);
        mOptionModel->setData( insertNumberIndex, -1, Qt::EditRole);
    }
    connect(mOptionModel, &QAbstractTableModel::dataChanged, mOptionModel, &SolverOptionTableModel::on_updateOptionItem, Qt::UniqueConnection);
    updateTableColumnSpan();
    setModified(true);
    emit itemCountChanged(mOptionModel->rowCount());

    ui->definitionTreeView->clearSelection();
    ui->optionTableView->clearSelection();
    ui->optionTableView->edit( mOptionModel->index(rowToBeInserted, mOptionModel->column_key()));
}

void SolverOptionEditor::updateTableColumnSpan()
{
    ui->optionTableView->clearSpans();
    QList<SolverOptionItem *> optionItems = mOptionModel->getCurrentListOfOptionItems();
    for(int i=0; i< optionItems.size(); ++i) {
        if (optionItems.at(i)->disabled)
            ui->optionTableView->setSpan(i, mOptionModel->column_key(), 1, mOptionModel->columnCount()-1);
    }
}

void SolverOptionEditor::insertComment()
{
    if  (isViewCompact())
        return;

    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        if (mOptionCompleter->currentEditedIndex().isValid() && mOptionCompleter->currentEditedIndex().row()==index.row())
            return;
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    disconnect(mOptionModel, &QAbstractTableModel::dataChanged, mOptionModel, &SolverOptionTableModel::on_updateOptionItem);
    int rowToBeInserted = -1;
    if (isThereARowSelection() ) {
        QList<int> rows;
        const auto indexes = ui->optionTableView->selectionModel()->selectedRows();
        for(const QModelIndex &idx : indexes) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        rowToBeInserted = rows.at(0);
        mOptionModel->insertRows(rowToBeInserted, 1, QModelIndex());
        const QModelIndex insertKeyIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_key());
        const QModelIndex insertValueIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_value());
        const QModelIndex insertNumberIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_id());

        mOptionModel->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                          Qt::CheckState(Qt::PartiallyChecked),
                                                          Qt::CheckStateRole );
        mOptionModel->setData( insertKeyIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        mOptionModel->setData( insertValueIndex, "", Qt::EditRole);
        mOptionModel->setData( insertNumberIndex, -1, Qt::EditRole);
    } else {
        mOptionModel->insertRows(mOptionModel->rowCount(), 1, QModelIndex());
        rowToBeInserted = mOptionModel->rowCount()-1;
        const QModelIndex insertKeyIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_key());
        const QModelIndex insertValueIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_value());
        const QModelIndex insertNumberIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_id());
        mOptionModel->setHeaderData(rowToBeInserted, Qt::Vertical,
                                                          Qt::CheckState(Qt::PartiallyChecked),
                                                          Qt::CheckStateRole );

        mOptionModel->setData( insertKeyIndex, OptionTokenizer::commentGeneratedStr, Qt::EditRole);
        mOptionModel->setData( insertValueIndex, "", Qt::EditRole);
//        if (mOptionModel->column_id() > mOptionModel->column_eol_comment()) {
        if (mOptionTokenizer->getOption()->isEOLCharDefined()) {
            const QModelIndex eolCommentIndex = mOptionModel->index(rowToBeInserted, mOptionModel->column_eol_comment());
            mOptionModel->setData( eolCommentIndex, "", Qt::EditRole);
        }
        mOptionModel->setData( insertNumberIndex, -1, Qt::EditRole);
    }
    connect(mOptionModel, &QAbstractTableModel::dataChanged, mOptionModel, &SolverOptionTableModel::on_updateOptionItem, Qt::UniqueConnection);
    updateTableColumnSpan();
    setModified(true);
    emit itemCountChanged(mOptionModel->rowCount());

    ui->definitionTreeView->clearSelection();
    ui->optionTableView->clearSelection();
    ui->optionTableView->edit(mOptionModel->index(rowToBeInserted, mOptionModel->column_key()));

}

void SolverOptionEditor::deleteCommentsBeforeOption(int row)
{
    QList<int> rows;
    for(int r=row-1; r>=0; r--) {
        if (mOptionModel->headerData(r, Qt::Vertical, Qt::CheckStateRole).toUInt()==Qt::PartiallyChecked) {
            rows.append( r );
        } else {
            break;
        }
    }

    std::sort(rows.begin(), rows.end());

    int prev = -1;
    for(int i=rows.count()-1; i>=0; i--) {
        const int current = rows[i];
        if (current != prev) {
            const QString text = mOptionModel->getOptionTableEntry(current);
            mOptionModel->removeRows( current, 1, QModelIndex() );
            mOptionTokenizer->logger()->append(QString("Option entry '%1' has been deleted").arg(text), LogMsgType::Info);
            prev = current;
        }
    }
    updateTableColumnSpan();
}

void SolverOptionEditor::deleteOption()
{
    if (isEditing()) {
        QLineEdit* editor = qobject_cast<QLineEdit *>(mOptionCompleter->lastEditor());
        if (editor) emit editor->editingFinished();
    }

    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }
    if  (!isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    if (isThereARowSelection() && isEverySelectionARow()) {
        const QItemSelection selection( ui->optionTableView->selectionModel()->selection() );

        QList<int> rows;
        const auto indexes = ui->optionTableView->selectionModel()->selectedRows();
        for(const QModelIndex & index : indexes) {
            rows.append( index.row() );
        }

        Settings* settings = Settings::settings();
        if (settings && settings->toBool(skSoDeleteCommentsAbove)) {
            const auto indexes = ui->optionTableView->selectionModel()->selectedRows();
            for(const QModelIndex & index : indexes) {
                if (mOptionModel->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt()!=Qt::PartiallyChecked) {
                    for(int row=index.row()-1; row>=0; row--) {
                        if (mOptionModel->headerData(row, Qt::Vertical, Qt::CheckStateRole).toUInt()==Qt::PartiallyChecked) {
                            rows.append( row );
                        } else {
                            break;
                        }
                    }
                }
            }
        }

        std::sort(rows.begin(), rows.end());
        int prev = -1;
        for(int i=rows.count()-1; i>=0; i--) {
            int current = rows[i];
            if (current != prev) {
                const QString text = mOptionModel->getOptionTableEntry(current);
                mOptionModel->removeRows( current, 1, QModelIndex() );
                mOptionTokenizer->logger()->append(QString("Option entry '%1' has been deleted").arg(text), LogMsgType::Info);
                prev = current;
            }
        }
        updateTableColumnSpan();
        setModified(true);
        emit itemCountChanged(mOptionModel->rowCount());
    }
}

void SolverOptionEditor::moveOptionUp()
{
    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if  (idxSelection.first().row() <= 0)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        mOptionModel->moveRows(QModelIndex(), idx.row(), 1,
                                                     QModelIndex(), idx.row()-1);
    }
    //    mOptionModel->moveRows(QModelIndex(), index.row(), selection.count(),
    //                                                 QModelIndex(), index.row()-1);
    updateTableColumnSpan();
    setModified(true);
}

void SolverOptionEditor::moveOptionDown()
{
    QModelIndexList indexSelection = ui->optionTableView->selectionModel()->selectedIndexes();
    for(const QModelIndex &index : std::as_const(indexSelection)) {
        ui->optionTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    if  (isViewCompact() || !isThereARow() || !isThereARowSelection() || !isEverySelectionARow())
        return;

    const QModelIndexList selection = ui->optionTableView->selectionModel()->selectedRows();
    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() > b.row(); });
    if  (idxSelection.first().row() >= mOptionModel->rowCount()-1)
        return;

    for(int i=0; i<idxSelection.size(); i++) {
        const QModelIndex idx = idxSelection.at(i);
        mOptionModel->moveRows(QModelIndex(), idx.row(), 1,
                                    QModelIndex(), idx.row()+2);
    }
    //    mOptionModel->moveRows(QModelIndex(), index.row(), selection.count(),
    //                                QModelIndex(), index.row()+selection.count()+1);
    updateTableColumnSpan();
    setModified(true);
}

bool SolverOptionEditor::isEditing()
{
    return (mOptionCompleter->lastEditor() && !mOptionCompleter->isLastEditorClosed());
}

void SolverOptionEditor::refreshOptionTableModel(bool hideAllComments)
{
    if (hideAllComments) {
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
            ui->optionTableView->hideColumn(mOptionModel->column_eol_comment());
    } else {
        if (mOptionTokenizer->getOption()->isEOLCharDefined())
            ui->optionTableView->showColumn(mOptionModel->column_eol_comment());
    }
    for(int i = 0; i < mOptionModel->rowCount(); ++i) {
        if (mOptionModel->headerData(i, Qt::Vertical, Qt::CheckStateRole).toUInt()==Qt::PartiallyChecked) {
            if (hideAllComments)
                ui->optionTableView->hideRow(i);
            else
                ui->optionTableView->showRow(i);
        }
    }
}

void SolverOptionEditor::addOptionModelFromDefinition(int row, const QModelIndex &descriptionIndex)
{
    if (row == definitionModel()->rowCount()) {
        mOptionModel->insertRows(row, 1, QModelIndex());
    }

    const QModelIndex parentIndex     =  mDefinitionModel->parent(descriptionIndex);
    const QModelIndex optionNameIndex = (parentIndex.row()<0) ? mDefinitionModel->index(descriptionIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                              : mDefinitionModel->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    const QModelIndex defValueIndex   = (parentIndex.row()<0) ? mDefinitionModel->index(descriptionIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE)
                                                              : mDefinitionModel->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    const QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex
                                                                 : mDefinitionModel->index(descriptionIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex) ;
    const QString selectedValueData = mDefinitionModel->data(selectedValueIndex, Qt::DisplayRole).toString();
    const QString optionNameData    = mDefinitionModel->data(optionNameIndex, Qt::DisplayRole).toString();

    const QModelIndex insertKeyIndex    = mOptionModel->index(row, SolverOptionTableModel::COLUMN_KEY);
    const QModelIndex insertValueIndex  = mOptionModel->index(row, SolverOptionTableModel::COLUMN_VALUE);
    const QModelIndex insertNumberIndex = mOptionModel->index(row, SolverOptionTableModel::COLUMN_ID);

    mOptionModel->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    mOptionModel->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    Settings* settings = Settings::settings();
    if (settings && settings->toBool(skSoAddEOLComment) && mOptionTokenizer->getOption()->isEOLCharDefined()) {
        const QModelIndex commentIndex = descriptionIndex.siblingAtColumn(OptionDefinitionModel::COLUMN_DESCIPTION);
        QString commentData            = mOptionModel->data(commentIndex).toString();
        QModelIndex insertEOLCommentIndex = mOptionModel->index(row, SolverOptionTableModel::COLUMN_EOL_COMMENT);
        mOptionModel->setData( insertEOLCommentIndex, commentData, Qt::EditRole);
    } else {
        QModelIndex insertEOLCommentIndex = mOptionModel->index(row, SolverOptionTableModel::COLUMN_EOL_COMMENT);
        mOptionModel->setData( insertEOLCommentIndex, "", Qt::EditRole);
    }
    const int optionEntryNumber = mOptionTokenizer->getOption()->getOptionDefinition(optionNameData).number;
    mOptionModel->setData( insertNumberIndex, optionEntryNumber, Qt::EditRole);
    mOptionModel->setHeaderData( row, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
}

void SolverOptionEditor::addCommentModelFromDefinition(int row, const QModelIndex &descriptionIndex)
{
    QString descriptionData = definitionModel()->data(descriptionIndex, Qt::DisplayRole).toString();

    mDefinitionModel->insertRows(row, 1, QModelIndex());

    QModelIndex insertKeyIndex    = mDefinitionModel->index(row, mOptionModel->column_key());
    QModelIndex insertValueIndex  = mDefinitionModel->index(row, mOptionModel->column_value());
    QModelIndex insertNumberIndex = mDefinitionModel->index(row, mOptionModel->column_id());

    mDefinitionModel->setHeaderData( insertKeyIndex.row(), Qt::Vertical,
                                      Qt::CheckState(Qt::PartiallyChecked),
                                      Qt::CheckStateRole );
    mDefinitionModel->setData( insertKeyIndex,    descriptionData, Qt::EditRole);
    mDefinitionModel->setData( insertValueIndex,  "",              Qt::EditRole);
    mDefinitionModel->setData( insertNumberIndex, -1,              Qt::EditRole);
}

void SolverOptionEditor::addEOLCommentModelFromDefinition(int row, const QModelIndex &selectedValueIndex,
                                                                   const QModelIndex &descriptionIndex)
{
    const QString selectedValueData = mDefinitionModel->data(selectedValueIndex, Qt::DisplayRole).toString();
    QString strData                 = selectedValueData;
    strData.append( " - " );
    strData.append( mDefinitionModel->data(descriptionIndex, Qt::DisplayRole).toString() );
    mDefinitionModel->insertRows(row, 1, QModelIndex());

    QModelIndex insertNumberIndex = definitionModel()->index(row, mOptionModel->column_id());
    QModelIndex insertKeyIndex    = definitionModel()->index(row, mOptionModel->column_key());
    QModelIndex insertValueIndex  = definitionModel()->index(row, mOptionModel->column_value());

    definitionModel()->setHeaderData( insertKeyIndex.row(), Qt::Vertical,
                                      Qt::CheckState(Qt::PartiallyChecked),
                                      Qt::CheckStateRole );
    definitionModel()->setData( insertNumberIndex, -1,      Qt::EditRole);
    definitionModel()->setData( insertKeyIndex,    strData, Qt::EditRole);
    definitionModel()->setData( insertValueIndex,  "",      Qt::EditRole);
}



} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams
