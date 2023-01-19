/* This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include <QMessageBox>
#include <QShortcut>
#include <QSslSocket>

#include "parametereditor.h"
#include "ui_parametereditor.h"

#include "headerviewproxy.h"
#include "addoptionheaderview.h"
#include "commonpaths.h"
#include "definitionitemdelegate.h"
#include "optionsortfilterproxymodel.h"
#include "gamsoptiondefinitionmodel.h"
#include "mainwindow.h"

namespace gams {
namespace studio {
namespace option {

ParameterEditor::ParameterEditor(QAction *aRun, QAction *aRunGDX, QAction *aCompile, QAction *aCompileGDX, QAction *aRunNeos,
                                 QAction *aRunEngine, QAction *aInterrupt, QAction *aStop, MainWindow *parent):
    QWidget(parent), ui(new Ui::ParameterEditor), actionRun(aRun), actionRun_with_GDX_Creation(aRunGDX),
    actionCompile(aCompile), actionCompile_with_GDX_Creation(aCompileGDX), actionRunNeos(aRunNeos),
    actionRunEngine(aRunEngine), actionInterrupt(aInterrupt), actionStop(aStop), main(parent)
{
    ui->setupUi(this);

    addActions();
    mOptionTokenizer = new OptionTokenizer(QString("optgams.def"));

    setRunsActionGroup(actionRun, actionRun_with_GDX_Creation, actionCompile, actionCompile_with_GDX_Creation,
                       actionRunNeos, actionRunEngine);
    setInterruptActionGroup(actionInterrupt, actionStop);

    setFocusPolicy(Qt::StrongFocus);

    connect(ui->gamsParameterCommandLine, &CommandLine::parameterRunChanged, main, &MainWindow::parameterRunChanged, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &QComboBox::editTextChanged, ui->gamsParameterCommandLine, &CommandLine::validateChangedParameter, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, mOptionTokenizer, &OptionTokenizer::formatTextLineEdit, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, this, &ParameterEditor::updateParameterTableModel, Qt::UniqueConnection );
    connect(ui->gamsParameterCommandLine, &CommandLine::parameterEditCancelled, this, &CommandLine::clearFocus, Qt::UniqueConnection);

    QList<OptionItem> optionItem = mOptionTokenizer->tokenize(ui->gamsParameterCommandLine->lineEdit()->text());
    QString normalizedText = mOptionTokenizer->normalize(optionItem);
    mParameterTableModel = new GamsParameterTableModel(normalizedText, mOptionTokenizer, this);
    ui->gamsParameterTableView->setModel( mParameterTableModel );
    connect(mParameterTableModel, &GamsParameterTableModel::optionModelChanged, this, static_cast<void(ParameterEditor::*)(const QList<OptionItem> &)> (&ParameterEditor::updateCommandLineStr),  Qt::UniqueConnection);
    connect(this, static_cast<void(ParameterEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&ParameterEditor::commandLineChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit, Qt::UniqueConnection);

    mOptionCompleter = new OptionCompleterDelegate(mOptionTokenizer, ui->gamsParameterTableView);
    ui->gamsParameterTableView->setItemDelegate( mOptionCompleter );
    connect(mOptionCompleter, &QStyledItemDelegate::commitData, this, &ParameterEditor::parameterItemCommitted);
    ui->gamsParameterTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::SelectedClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui->gamsParameterTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->gamsParameterTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->gamsParameterTableView->setAutoScroll(true);
    ui->gamsParameterTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->gamsParameterTableView->setSortingEnabled(false);

    ui->gamsParameterTableView->setDragEnabled(true);
    ui->gamsParameterTableView->viewport()->setAcceptDrops(true);
    ui->gamsParameterTableView->setDropIndicatorShown(true);
    ui->gamsParameterTableView->setDragDropMode(QAbstractItemView::DropOnly);
    ui->gamsParameterTableView->setDragDropOverwriteMode(true);
    ui->gamsParameterTableView->setDefaultDropAction(Qt::CopyAction);

    AddOptionHeaderView* headerView = new AddOptionHeaderView(Qt::Horizontal, ui->gamsParameterTableView);
    headerView->setSectionResizeMode(QHeaderView::Interactive);
    QFontMetrics fm(QFont("times", 16));
    headerView->setMinimumSectionSize(fm.horizontalAdvance("Key/Value"));

    ui->gamsParameterTableView->setHorizontalHeader(headerView);
    ui->gamsParameterTableView->setColumnHidden(GamsParameterTableModel::COLUMN_ENTRY_NUMBER, true);
    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->gamsParameterTableView->horizontalHeader()->setStyle(HeaderViewProxy::instance());

    ui->gamsParameterTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->gamsParameterTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));
    ui->gamsParameterTableView->horizontalHeader()->setStretchLastSection(true);
    ui->gamsParameterTableView->verticalHeader()->setMinimumSectionSize(1);
    ui->gamsParameterTableView->verticalHeader()->setDefaultSectionSize(int(fontMetrics().height()*TABLE_ROW_HEIGHT));

    connect(ui->gamsParameterTableView, &QTableView::customContextMenuRequested,this, &ParameterEditor::showParameterContextMenu, Qt::UniqueConnection);
    connect(this, &ParameterEditor::ParameterTableModelChanged, this, &ParameterEditor::on_parameterTableModelChanged, Qt::UniqueConnection);
    connect(mParameterTableModel, &GamsParameterTableModel::newTableRowDropped, this, &ParameterEditor::on_newTableRowDropped, Qt::UniqueConnection);
    connect(mParameterTableModel, &GamsParameterTableModel::optionNameChanged, this, &ParameterEditor::on_parameterTableNameChanged, Qt::UniqueConnection);
    connect(mParameterTableModel, &GamsParameterTableModel::optionValueChanged, this, &ParameterEditor::on_parameterValueChanged, Qt::UniqueConnection);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    GamsOptionDefinitionModel* optdefmodel =  new GamsOptionDefinitionModel(mOptionTokenizer->getOption(), 0, this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->gamsParameterSearch, &FilterLineEdit::regExpChanged, proxymodel, [this, proxymodel]() {
        proxymodel->setFilterRegExp(ui->gamsParameterSearch->regExp());
    });

    if (HeaderViewProxy::platformShouldDrawBorder())
        ui->gamsParameterTreeView->header()->setStyle(HeaderViewProxy::instance());
    ui->gamsParameterTreeView->setModel( proxymodel );
    ui->gamsParameterTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->gamsParameterTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->gamsParameterTreeView->setDragEnabled(true);
    ui->gamsParameterTreeView->setDragDropMode(QAbstractItemView::DragOnly);

    ui->gamsParameterTreeView->setItemDelegate( new DefinitionItemDelegate(ui->gamsParameterTreeView) );
    ui->gamsParameterTreeView->setItemsExpandable(true);
    ui->gamsParameterTreeView->setSortingEnabled(true);
    ui->gamsParameterTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
    ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
    ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
    ui->gamsParameterTreeView->setExpandsOnDoubleClick(false);
    ui->gamsParameterTreeView->setColumnHidden(OptionDefinitionModel::COLUMN_ENTRY_NUMBER, true);
    ui->gamsParameterTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
    connect(ui->gamsParameterTreeView, &QTreeView::customContextMenuRequested, this, &ParameterEditor::showDefinitionContextMenu, Qt::UniqueConnection);
    connect(ui->gamsParameterTreeView, &QAbstractItemView::doubleClicked, this, &ParameterEditor::addParameterFromDefinition, Qt::UniqueConnection);

    connect(mParameterTableModel, &GamsParameterTableModel::optionModelChanged, optdefmodel, &GamsOptionDefinitionModel::modifyOptionDefinition, Qt::UniqueConnection);

    mExtendedEditor = new QDockWidget("GAMS Parameters", this);
    mExtendedEditor->setObjectName("gamsArguments");

    mDockChild = new AbstractView(mExtendedEditor);
    mExtendedEditor->setWidget(mDockChild);
    QVBoxLayout *lay = new QVBoxLayout(mDockChild);
    lay->addWidget(ui->gamsParameterEditor);
    lay->setContentsMargins(0,0,0,0);
    mDockChild->setLayout(lay);

    mExtendedEditor->setFeatures(QDockWidget::NoDockWidgetFeatures);
    mExtendedEditor->setTitleBarWidget(new QWidget(this));
    main->addDockWidget(Qt::TopDockWidgetArea, mExtendedEditor);
    connect(mExtendedEditor, &QDockWidget::visibilityChanged, main, &MainWindow::setExtendedEditorVisibility, Qt::UniqueConnection);
    mExtendedEditor->setVisible(false);

#ifdef __APPLE__
    ui->verticalLayout->setContentsMargins(2,2,2,0);
#else
    ui->verticalLayout->setContentsMargins(2,0,2,2);
#endif
}

ParameterEditor::~ParameterEditor()
{
    delete ui;
    if (mOptionTokenizer)
       delete mOptionTokenizer;
    if (mParameterTableModel)
       delete mParameterTableModel;
    if (mOptionCompleter)
       delete mOptionCompleter;
}

void ParameterEditor::runDefaultAction()
{
    ui->gamsRunToolButton->defaultAction()->trigger();
}

QString ParameterEditor::on_runAction(RunActionState state)
{
    QString commandLineStr =  ui->gamsParameterCommandLine->getParameterString();

    if (!commandLineStr.endsWith(" "))
        commandLineStr.append(" ");

    bool gdxParam = false;
    bool actParam = false;
    for (const option::OptionItem &item : getOptionTokenizer()->tokenize( commandLineStr)) {
        if (QString::compare(item.key, "gdx", Qt::CaseInsensitive) == 0)
            gdxParam = true;
        if ((QString::compare(item.key, "action", Qt::CaseInsensitive) == 0) ||
            (QString::compare(item.key, "a", Qt::CaseInsensitive) == 0))
            actParam = true;
    }

    if (state == RunActionState::RunWithGDXCreation) {
       if (!gdxParam) commandLineStr.append("GDX=default");
       ui->gamsRunToolButton->setDefaultAction( actionRun_with_GDX_Creation );

    } else if (state == RunActionState::Compile) {
        if (!actParam) commandLineStr.append("ACTION=C");
        ui->gamsRunToolButton->setDefaultAction( actionCompile );

    } else if (state == RunActionState::CompileWithGDXCreation) {
        if (!gdxParam) commandLineStr.append("GDX=default ");
        if (!actParam) commandLineStr.append("ACTION=C");
        ui->gamsRunToolButton->setDefaultAction( actionCompile_with_GDX_Creation );

    } else if (state == RunActionState::RunNeos) {
        ui->gamsRunToolButton->setDefaultAction( actionRunNeos );

    } else if (state == RunActionState::RunEngine) {
        ui->gamsRunToolButton->setDefaultAction( actionRunEngine );

    } else {
        ui->gamsRunToolButton->setDefaultAction( actionRun );
    }

    return commandLineStr.simplified();
}

void ParameterEditor::on_interruptAction()
{
    ui->gamsInterruptToolButton->setDefaultAction( actionInterrupt );
}

void ParameterEditor::on_stopAction()
{
    ui->gamsInterruptToolButton->setDefaultAction( actionStop );
}

AbstractView *ParameterEditor::dockChild()
{
    return mDockChild;
}

void ParameterEditor::updateParameterTableModel(QLineEdit *lineEdit, const QString &commandLineStr)
{
    Q_UNUSED(lineEdit)
    if (mExtendedEditor->isHidden()) return;

    emit ParameterTableModelChanged(commandLineStr);
}

void ParameterEditor::updateCommandLineStr(const QList<OptionItem> &optionItems)
{
    if (mDockChild->isHidden())
       return;

    emit commandLineChanged(ui->gamsParameterCommandLine->lineEdit(), optionItems);
}

void ParameterEditor::showParameterContextMenu(const QPoint &pos)
{
    QModelIndexList indexSelection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsParameterTableView->selectionModel()->selectedRows();
    bool thereIsARowSelection = (selection.count() > 0);
    bool thereIsARow = (ui->gamsParameterTableView->model()->rowCount() > 0);

    QMenu menu(this);
    for(QAction* action : ui->gamsParameterTableView->actions()) {
        if (action->objectName().compare("actionInsert_option")==0) {
            if (!thereIsARow || thereIsARowSelection)
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDelete_option")==0) {
                  if ( thereIsARowSelection )
                     menu.addAction(action);
        } else if (action->objectName().compare("actionDeleteAll_option")==0) {
                 if (thereIsARow)
                    menu.addAction(action);
                 menu.addSeparator();
        } else if (action->objectName().compare("actionMoveUp_option")==0) {
                 if (thereIsARowSelection && (selection.first().row() > 0))
                    menu.addAction(action);
        } else if (action->objectName().compare("actionMoveDown_option")==0) {
                  if (thereIsARowSelection && (selection.last().row() < mParameterTableModel->rowCount()-1) )
                     menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionSelect_all")==0) {
                  if (thereIsARow)
                     menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionShowDefinition_option")==0) {

            bool thereIsAnOptionSelection = false;
            for (QModelIndex s : qAsConst(selection)) {
                QVariant data = ui->gamsParameterTableView->model()->headerData(s.row(), Qt::Vertical,  Qt::CheckStateRole);
                if (Qt::CheckState(data.toUInt())!=Qt::PartiallyChecked) {
                    thereIsAnOptionSelection = true;
                    break;
                }
            }
            if (thereIsAnOptionSelection)
                menu.addAction(action);
        } else if (action->objectName().compare("actionShowRecurrence_option")==0) {
                  if ( indexSelection.size()>=1 && getRecurrentParameter(indexSelection.at(0)).size()>0 )
                      menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionResize_columns")==0) {
                  if (thereIsARow)
                     menu.addAction(action);
        }
    }
    menu.exec(ui->gamsParameterTableView->viewport()->mapToGlobal(pos));
}

void ParameterEditor::showDefinitionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = ui->gamsParameterTreeView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    bool hasSelectionBeenAdded = (selection.size()>0);
    // assume single selection
    for (QModelIndex idx : qAsConst(selection)) {
        QModelIndex parentIdx = ui->gamsParameterTreeView->model()->parent(idx);
        QVariant data = (parentIdx.row() < 0) ? ui->gamsParameterTreeView->model()->data(idx, Qt::CheckStateRole)
                                              : ui->gamsParameterTreeView->model()->data(parentIdx, Qt::CheckStateRole);
        hasSelectionBeenAdded = (Qt::CheckState(data.toInt()) == Qt::Checked);
    }

    QMenu menu(this);
    for(QAction* action : ui->gamsParameterTreeView->actions()) {
        if (action->objectName().compare("actionAddThisOption")==0) {
            if ( !hasSelectionBeenAdded && ui->gamsParameterTableView->selectionModel()->selectedRows().size() <= 0)
                menu.addAction(action);
            menu.addSeparator();
        } else if (action->objectName().compare("actionDeleteThisOption")==0) {
                  if ( hasSelectionBeenAdded && ui->gamsParameterTableView->selectionModel()->selectedRows().size() > 0 )
                     menu.addAction(action);
                  menu.addSeparator();
        } else if (action->objectName().compare("actionResize_columns")==0) {
                  if ( ui->gamsParameterTreeView->model()->rowCount()>0 )
                     menu.addAction(action);
                  menu.addSeparator();
        }
    }

    menu.exec(ui->gamsParameterTreeView->viewport()->mapToGlobal(pos));

}

void ParameterEditor::updateRunState(bool isRunnable, bool isRunning)
{
    bool activate = isRunnable && !isRunning;
    setRunActionsEnabled(activate);
    setInterruptActionsEnabled(isRunnable && isRunning);

    mDockChild->setEnabled(activate);
    ui->gamsParameterCommandLine->setEnabled(activate && !isEditorExtended());
}

void ParameterEditor::addParameterFromDefinition(const QModelIndex &index)
{
    QModelIndex parentIndex =  ui->gamsParameterTreeView->model()->parent(index);
    QModelIndex optionNameIndex = (parentIndex.row()<0) ? ui->gamsParameterTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) :
                                                          ui->gamsParameterTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ;
    QModelIndex defValueIndex = (parentIndex.row()<0) ? ui->gamsParameterTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) :
                                                        ui->gamsParameterTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_DEF_VALUE) ;
    QModelIndex entryNumberIndex = (parentIndex.row()<0) ? ui->gamsParameterTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) :
                                                           ui->gamsParameterTreeView->model()->index(parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER) ;
    QModelIndex selectedValueIndex = (parentIndex.row()<0) ? defValueIndex :
                                                             ui->gamsParameterTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex) ;

    QString optionNameData = ui->gamsParameterTreeView->model()->data(optionNameIndex).toString();

    QString selectedValueData = ui->gamsParameterTreeView->model()->data(selectedValueIndex).toString();
    QString entryNumberData = ui->gamsParameterTreeView->model()->data(entryNumberIndex).toString();

    QModelIndexList indices = ui->gamsParameterTableView->model()->match(ui->gamsParameterTableView->model()->index(GamsParameterTableModel::COLUMN_OPTION_KEY, GamsParameterTableModel::COLUMN_ENTRY_NUMBER),
                                                                     Qt::DisplayRole,
                                                                     entryNumberData, -1, Qt::MatchExactly|Qt::MatchRecursive);
    ui->gamsParameterTableView->clearSelection();
    QItemSelection selection;
    for(QModelIndex &idx: indices) {
        QModelIndex leftIndex  = ui->gamsParameterTableView->model()->index(idx.row(), GamsParameterTableModel::COLUMN_OPTION_KEY);
        QModelIndex rightIndex = ui->gamsParameterTableView->model()->index(idx.row(), GamsParameterTableModel::COLUMN_ENTRY_NUMBER);
        QItemSelection rowSelection(leftIndex, rightIndex);
        selection.merge(rowSelection, QItemSelectionModel::Select);
    }
    ui->gamsParameterTableView->selectionModel()->select(selection, QItemSelectionModel::Select);
    int rowToBeAdded = ui->gamsParameterTableView->model()->rowCount();

    bool singleEntryExisted = (indices.size()==1);
    bool multipleEntryExisted = (indices.size()>1);
    if (singleEntryExisted ) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Parameter Entry exists");
        msgBox.setText("Parameter '" + optionNameData+ "' already exists.");
        msgBox.setInformativeText("How do you want to proceed?");
        msgBox.setDetailedText(QString("Entry:  '%1'\nDescription:  %2 %3").arg(getParameterTableEntry(indices.at(0).row()))
                .arg("When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by GAMS.",
                     "The value of all other entries except the last entry will be ignored."));
        msgBox.setStandardButtons(QMessageBox::Abort);
        msgBox.addButton("Replace existing entry", QMessageBox::ActionRole);
        msgBox.addButton("Add new entry", QMessageBox::ActionRole);

        switch(msgBox.exec()) {
        case 0: // replace
            rowToBeAdded = indices.at(0).row();
            break;
        case 1: // add
            break;
        case QMessageBox::Abort:
            return;
        }
    } else if (multipleEntryExisted) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Multiple Parameter Entries exist");
        msgBox.setText(QString("%1 entries of Parameter '%2' already exist.").arg(indices.size()).arg(optionNameData));
        msgBox.setInformativeText("How do you want to proceed?");
        QString entryDetailedText = QString("Entries:\n");
        int i = 0;
        for (QModelIndex &idx : indices)
            entryDetailedText.append(QString("   %1. '%2'\n").arg(++i).arg(getParameterTableEntry(idx.row())));
        msgBox.setDetailedText(QString("%1Description:  %2 %3").arg(entryDetailedText)
                 .arg("When running GAMS with multiple entries of the same parameter, only the value of the last entry will be utilized by the GAMS.",
                      "The value of all other entries except the last entry will be ignored."));
        msgBox.setText("Multiple entries of Parameter '" + optionNameData + "' already exist.");
        msgBox.setInformativeText("How do you want to proceed?");
        msgBox.setStandardButtons(QMessageBox::Abort);
        msgBox.addButton("Replace first entry and delete other entries", QMessageBox::ActionRole);
        msgBox.addButton("Add new entry", QMessageBox::ActionRole);

        switch(msgBox.exec()) {
        case 0: { // delete and replace
            QList<int> overrideIdRowList;
            for(QModelIndex idx : qAsConst(indices)) { overrideIdRowList.append(idx.row()); }
            std::sort(overrideIdRowList.begin(), overrideIdRowList.end());

            rowToBeAdded = overrideIdRowList.at(0);
            QItemSelection selection;
            QModelIndex leftIndex  = ui->gamsParameterTableView->model()->index(rowToBeAdded, GamsParameterTableModel::COLUMN_OPTION_KEY);
            QModelIndex rightIndex = ui->gamsParameterTableView->model()->index(rowToBeAdded, GamsParameterTableModel::COLUMN_ENTRY_NUMBER);
            QItemSelection rowSelection(leftIndex, rightIndex);
            selection.merge(rowSelection, QItemSelectionModel::Deselect);
            deleteParameter();
            break;
        }
        case 1: { /* add */  break; }
        case QMessageBox::Abort: { return; }
        }
    } // else entry not exist*/

    ui->gamsParameterTableView->selectionModel()->clearSelection();
    if (rowToBeAdded == ui->gamsParameterTableView->model()->rowCount()) {
        ui->gamsParameterTableView->model()->insertRows(rowToBeAdded, 1, QModelIndex());
    }
    QModelIndex insertKeyIndex = ui->gamsParameterTableView->model()->index(rowToBeAdded, 0);
    QModelIndex insertValueIndex = ui->gamsParameterTableView->model()->index(rowToBeAdded, 1);
    QModelIndex insertEntryIndex = ui->gamsParameterTableView->model()->index(rowToBeAdded, 2);
    ui->gamsParameterTableView->model()->setData( insertKeyIndex, optionNameData, Qt::EditRole);
    ui->gamsParameterTableView->model()->setData( insertValueIndex, selectedValueData, Qt::EditRole);
    ui->gamsParameterTableView->model()->setData( insertEntryIndex, entryNumberData, Qt::EditRole);
    ui->gamsParameterTableView->selectionModel()->select( mParameterTableModel->index(ui->gamsParameterTableView->model()->rowCount()-1, 0),
                                                       QItemSelectionModel::Select|QItemSelectionModel::Rows );
    ui->gamsParameterTableView->model()->setData( insertEntryIndex, entryNumberData, Qt::EditRole);
    ui->gamsParameterTableView->model()->setHeaderData( rowToBeAdded, Qt::Vertical, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole );
    ui->gamsParameterTableView->selectRow(rowToBeAdded);

    showParameterDefinition();

    if (parentIndex.row()<0)
        ui->gamsParameterTableView->edit(insertValueIndex);
}

void ParameterEditor::loadCommandLine(const QStringList &history)
{
    // disconnect
    disconnect(ui->gamsParameterCommandLine, &QComboBox::editTextChanged,
            ui->gamsParameterCommandLine, &CommandLine::validateChangedParameter);
    disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            mOptionTokenizer, &OptionTokenizer::formatTextLineEdit);
    disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            this, &ParameterEditor::updateParameterTableModel );
    disconnect(mParameterTableModel, &GamsParameterTableModel::optionModelChanged, this, static_cast<void(ParameterEditor::*)(const QList<OptionItem> &)> (&ParameterEditor::updateCommandLineStr));
    disconnect(this, static_cast<void(ParameterEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&ParameterEditor::commandLineChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);

    ui->gamsParameterTreeView->clearSelection();
    ui->gamsParameterTreeView->collapseAll();
    ui->gamsParameterCommandLine->clear();
    ui->gamsParameterCommandLine->resetCurrentValue();
    for (const QString &str: history) {
        ui->gamsParameterCommandLine->insertItem(0, str );
    }
    if (history.size() > 0) {
        ui->gamsParameterCommandLine->validateChangedParameter( history.last() );
    }

    connect(ui->gamsParameterCommandLine, &QComboBox::editTextChanged,
            ui->gamsParameterCommandLine, &CommandLine::validateChangedParameter, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            mOptionTokenizer, &OptionTokenizer::formatTextLineEdit, Qt::UniqueConnection);
    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            this, &ParameterEditor::updateParameterTableModel, Qt::UniqueConnection);
    connect(mParameterTableModel, &GamsParameterTableModel::optionModelChanged, this, static_cast<void(ParameterEditor::*)(const QList<OptionItem> &)> (&ParameterEditor::updateCommandLineStr), Qt::UniqueConnection);
    connect(this, static_cast<void(ParameterEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&ParameterEditor::commandLineChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit, Qt::UniqueConnection);

    if (history.isEmpty()) {
        ui->gamsParameterTreeView->clearSelection();
        ui->gamsParameterTreeView->collapseAll();
    }
    ui->gamsParameterCommandLine->setCurrentIndex(0);
    emit ui->gamsParameterCommandLine->commandLineChanged(ui->gamsParameterCommandLine->lineEdit(), ui->gamsParameterCommandLine->currentText());
}

void ParameterEditor::selectSearchField()
{
    ui->gamsParameterSearch->setFocus();
}

void ParameterEditor::parameterItemCommitted(QWidget *editor)
{
    Q_UNUSED(editor)
    if (mOptionCompleter->currentEditedIndex().isValid()) {
        ui->gamsParameterTableView->selectionModel()->select( mOptionCompleter->currentEditedIndex(), QItemSelectionModel::ClearAndSelect );
        ui->gamsParameterTableView->setCurrentIndex( mOptionCompleter->currentEditedIndex() );
        ui->gamsParameterTableView->setFocus();
    }
}

void ParameterEditor::deSelectParameters()
{
    if (ui->gamsParameterTableView->hasFocus() && ui->gamsParameterTableView->selectionModel()->hasSelection()) {
        ui->gamsParameterTableView->selectionModel()->clearSelection();
        ui->gamsParameterTreeView->selectionModel()->clearSelection();
    } else if (ui->gamsParameterTreeView->hasFocus() && ui->gamsParameterTreeView->selectionModel()->hasSelection()) {
             ui->gamsParameterTreeView->selectionModel()->clearSelection();
    } else {
        this->focusNextChild();
    }
}

void ParameterEditor::findAndSelectionParameterFromDefinition()
{
    if (ui->gamsParameterTableView->model()->rowCount() <= 0)
        return;

    QModelIndex index = ui->gamsParameterTreeView->selectionModel()->currentIndex();
    QModelIndex parentIndex =  ui->gamsParameterTreeView->model()->parent(index);

    QModelIndex idx = (parentIndex.row()<0) ? ui->gamsParameterTreeView->model()->index( index.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER )
                                            : ui->gamsParameterTreeView->model()->index( parentIndex.row(), OptionDefinitionModel::COLUMN_ENTRY_NUMBER );
    QVariant data = ui->gamsParameterTreeView->model()->data( idx, Qt::DisplayRole );
    QModelIndexList indices = ui->gamsParameterTableView->model()->match(ui->gamsParameterTableView->model()->index(0, GamsParameterTableModel::COLUMN_ENTRY_NUMBER),
                                                                       Qt::DisplayRole,
                                                                       data, -1, Qt::MatchExactly|Qt::MatchRecursive);
    ui->gamsParameterTableView->clearSelection();
    ui->gamsParameterTableView->clearFocus();
    QItemSelection selection;
    for(QModelIndex i :qAsConst(indices)) {
        QModelIndex valueIndex = ui->gamsParameterTableView->model()->index(i.row(), GamsParameterTableModel::COLUMN_OPTION_VALUE);
        QString value =  ui->gamsParameterTableView->model()->data( valueIndex, Qt::DisplayRole).toString();
        bool selected = false;
        if (parentIndex.row() < 0) {
            selected = true;
        } else {
            QModelIndex enumIndex = ui->gamsParameterTreeView->model()->index(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex);
            QString enumValue = ui->gamsParameterTreeView->model()->data( enumIndex, Qt::DisplayRole).toString();
            if (QString::compare(value, enumValue, Qt::CaseInsensitive)==0)
                selected = true;
        }
        if (selected) {
           QModelIndex leftIndex  = ui->gamsParameterTableView->model()->index(i.row(), 0);
           QModelIndex rightIndex = ui->gamsParameterTableView->model()->index(i.row(), ui->gamsParameterTableView->model()->columnCount() -1);

           QItemSelection rowSelection(leftIndex, rightIndex);
           selection.merge(rowSelection, QItemSelectionModel::Select);
        }
    }

    ui->gamsParameterTableView->selectionModel()->select(selection, QItemSelectionModel::Select);
    ui->gamsParameterTreeView->setFocus();
}

void ParameterEditor::showParameterDefinition()
{
   if (!mExtendedEditor->isVisible())
       return;

    QModelIndexList indexSelection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsParameterTableView->selectionModel()->selectedRows();

    if (selection.count() <= 0)
       return;

    disconnect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition);

    QModelIndexList selectIndices;
    for (int i=0; i<selection.count(); i++) {
            QModelIndex index = selection.at(i);
            if (Qt::CheckState(ui->gamsParameterTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole).toUInt())==Qt::PartiallyChecked)
                continue;

            QString value = ui->gamsParameterTableView->model()->data( index.sibling(index.row(), GamsParameterTableModel::COLUMN_OPTION_VALUE), Qt::DisplayRole).toString();
            QVariant optionId = ui->gamsParameterTableView->model()->data( index.sibling(index.row(), ui->gamsParameterTableView->model()->columnCount()-1), Qt::DisplayRole);
            QModelIndexList indices = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_ENTRY_NUMBER),
                                                                               Qt::DisplayRole,
                                                                               optionId, 1, Qt::MatchExactly|Qt::MatchRecursive);
            for(QModelIndex idx : qAsConst(indices)) {
                QModelIndex  parentIndex =  ui->gamsParameterTreeView->model()->parent(index);
                QModelIndex optionIdx = ui->gamsParameterTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);
                if (parentIndex.row() < 0) {
                    if (ui->gamsParameterTreeView->model()->hasChildren(optionIdx) && !ui->gamsParameterTreeView->isExpanded(optionIdx))
                        ui->gamsParameterTreeView->expand(optionIdx);
                }
                bool found = false;
                for(int r=0; r <ui->gamsParameterTreeView->model()->rowCount(optionIdx); ++r) {
                    QModelIndex i = ui->gamsParameterTreeView->model()->index(r, OptionDefinitionModel::COLUMN_OPTION_NAME, optionIdx);
                    QString enumValue = ui->gamsParameterTreeView->model()->data(i, Qt::DisplayRole).toString();
                    if (QString::compare(value, enumValue, Qt::CaseInsensitive) == 0) {
                        selectIndices << i;
                        found = true;
                        break;
                    }
                }
                if (!found)
                   selectIndices << optionIdx;
            }
    }
    ui->gamsParameterTreeView->selectionModel()->clearSelection();
    for(QModelIndex idx : qAsConst(selectIndices)) {
        QItemSelection selection = ui->gamsParameterTreeView->selectionModel()->selection();
        QModelIndex  parentIdx =  ui->gamsParameterTreeView->model()->parent(idx);
        if (parentIdx.row() < 0) {
            selection.select(ui->gamsParameterTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME),
                             ui->gamsParameterTreeView->model()->index(idx.row(), ui->gamsParameterTreeView->model()->columnCount()-1));
        } else  {
            selection.select(ui->gamsParameterTreeView->model()->index(idx.row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIdx),
                             ui->gamsParameterTreeView->model()->index(idx.row(), ui->gamsParameterTreeView->model()->columnCount()-1, parentIdx));
        }
        ui->gamsParameterTreeView->selectionModel()->select(selection, QItemSelectionModel::Select);
    }
    if (selectIndices.size() > 0) {
        QModelIndex parentIndex = ui->gamsParameterTreeView->model()->parent(selectIndices.first());
        QModelIndex scrollToIndex = (parentIndex.row() < 0  ? ui->gamsParameterTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME)
                                                            : ui->gamsParameterTreeView->model()->index(selectIndices.first().row(), OptionDefinitionModel::COLUMN_OPTION_NAME, parentIndex));
        ui->gamsParameterTreeView->scrollTo(scrollToIndex, QAbstractItemView::EnsureVisible);
        if (parentIndex.row() >= 0)  {
            ui->gamsParameterTreeView->scrollTo(parentIndex, QAbstractItemView::EnsureVisible);
            const QRect r = ui->gamsParameterTreeView->visualRect(parentIndex);
            ui->gamsParameterTreeView->horizontalScrollBar()->setValue(r.x());
        } else {
            const QRect r = ui->gamsParameterTreeView->visualRect(scrollToIndex);
            ui->gamsParameterTreeView->horizontalScrollBar()->setValue(r.x());
        }
    }
    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParameterEditor::showParameterRecurrence()
{
    if (!mExtendedEditor->isVisible()) {
        return;
    }

    QModelIndexList indexSelection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
    if (indexSelection.size() <= 0) {
        showParameterDefinition();
        return;
    }

    QItemSelection selection = ui->gamsParameterTableView->selectionModel()->selection();
    selection.select(ui->gamsParameterTableView->model()->index(indexSelection.at(0).row(), 0),
                     ui->gamsParameterTableView->model()->index(indexSelection.at(0).row(), GamsParameterTableModel::COLUMN_ENTRY_NUMBER));
    ui->gamsParameterTableView->selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows );

    QList<int> rowList = getRecurrentParameter(indexSelection.at(0));
    if (rowList.size() <= 0) {
        showParameterDefinition();
        return;
    }

    for(int row : qAsConst(rowList)) {
        QItemSelection rowSelection = ui->gamsParameterTableView->selectionModel()->selection();
        rowSelection.select(ui->gamsParameterTableView->model()->index(row, 0),
                            ui->gamsParameterTableView->model()->index(row, GamsParameterTableModel::COLUMN_ENTRY_NUMBER));
        ui->gamsParameterTableView->selectionModel()->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::Rows );
    }

    showParameterDefinition();
}

void ParameterEditor::deleteParameter()
{
    if (!mExtendedEditor->isVisible())
        return;

     QModelIndexList indexSelection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
     for(QModelIndex index : qAsConst(indexSelection)) {
         ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
     }

     QModelIndexList selection = ui->gamsParameterTableView->selectionModel()->selectedRows();
     if (selection.count() <= 0)
        return;

    disconnect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition);

    QModelIndex index = selection.at(0);
    QModelIndex removeTableIndex = ui->gamsParameterTableView->model()->index(index.row(), 0);
    QVariant optionName = ui->gamsParameterTableView->model()->data(removeTableIndex, Qt::DisplayRole);

    QModelIndexList items = ui->gamsParameterTableView->model()->match(ui->gamsParameterTableView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, -1);
    QModelIndexList definitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);

    QList<int> rows;
    for(const QModelIndex & index : ui->gamsParameterTableView->selectionModel()->selectedRows()) {
        rows.append( index.row() );
    }
    std::sort(rows.begin(), rows.end());
    int prev = -1;
    for(int i=rows.count()-1; i>=0; i--) {
        int current = rows[i];
        if (current != prev) {
            ui->gamsParameterTableView->model()->removeRows( current, 1 );
            prev = current;
        }
    }

    ui->gamsParameterTreeView->clearSelection();
    ui->gamsParameterTableView->setFocus();
    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParameterEditor::deleteAllParameters()
{
    if (!mExtendedEditor->isVisible() || !ui->gamsParameterTableView->hasFocus() || ui->gamsParameterTableView->model()->rowCount() <= 0)
        return;

    mOptionTokenizer->getOption()->resetModficationFlag();

    QModelIndexList items = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::CheckStateRole,
                                                                     Qt::CheckState(Qt::Checked),
                                                                     ui->gamsParameterTreeView->model()->rowCount());
    for(QModelIndex item : qAsConst(items)) {
        ui->gamsParameterTreeView->model()->setData(item, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
    }
    ui->gamsParameterTreeView->collapseAll();
    ui->gamsParameterTableView->model()->removeRows(0, ui->gamsParameterTableView->model()->rowCount(), QModelIndex());

    emit ParameterTableModelChanged("");
}

void ParameterEditor::insertParameter()
{
    if (!mExtendedEditor->isVisible() || !ui->gamsParameterTableView->hasFocus())
        return;

    QModelIndexList indexSelection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsParameterTableView->selectionModel()->selectedRows();

    if (mParameterTableModel->rowCount() <= 0 || selection.count() <= 0) {
        mParameterTableModel->insertRows(mParameterTableModel->rowCount(), 1, QModelIndex());
        QModelIndex index = mParameterTableModel->index( mParameterTableModel->rowCount()-1, GamsParameterTableModel::COLUMN_OPTION_KEY);
        ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
        ui->gamsParameterTableView->edit( index );

        ui->gamsParameterTableView->scrollTo(index, QAbstractItemView::EnsureVisible);
    } else if (selection.count() > 0) {
        QList<int> rows;
        for(QModelIndex idx : qAsConst(selection)) {
            rows.append( idx.row() );
        }
        std::sort(rows.begin(), rows.end());
        ui->gamsParameterTableView->model()->insertRows(rows.at(0), 1, QModelIndex());
        QModelIndex index = ui->gamsParameterTableView->model()->index(rows.at(0), GamsParameterTableModel::COLUMN_OPTION_KEY);
        ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
        ui->gamsParameterTableView->edit( mParameterTableModel->index(index.row(), GamsParameterTableModel::COLUMN_OPTION_KEY) );

        ui->gamsParameterTableView->scrollTo(index, QAbstractItemView::EnsureVisible);
    }
}

void ParameterEditor::moveParameterUp()
{
    QModelIndexList indexSelection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsParameterTableView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
       return;

    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() < b.row(); });
    if (idxSelection.first().row() <= 0)
       return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
        ui->gamsParameterTableView->model()->moveRows(QModelIndex(), idx.row(), 1,
                                                    QModelIndex(), idx.row()-1);
    }

    disconnect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition);
    QItemSelection select;
    for(QModelIndex indx : qAsConst(idxSelection)) {
        QModelIndex leftIndex  = ui->gamsParameterTableView->model()->index(indx.row()-1, GamsParameterTableModel::COLUMN_OPTION_KEY);
        QModelIndex rightIndex = ui->gamsParameterTableView->model()->index(indx.row()-1, GamsParameterTableModel::COLUMN_ENTRY_NUMBER);
        QItemSelection rowSelection(leftIndex, rightIndex);
        select.merge(rowSelection, QItemSelectionModel::Select);
    }
    ui->gamsParameterTableView->selectionModel()->select(select, QItemSelectionModel::ClearAndSelect);
    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParameterEditor::moveParameterDown()
{
    QModelIndexList indexSelection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
    for(QModelIndex index : qAsConst(indexSelection)) {
        ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

    QModelIndexList selection = ui->gamsParameterTableView->selectionModel()->selectedRows();
    if (selection.count() <= 0)
        return;

    QModelIndexList idxSelection = QModelIndexList(selection);
    std::stable_sort(idxSelection.begin(), idxSelection.end(), [](QModelIndex a, QModelIndex b) { return a.row() > b.row(); });
    if (idxSelection.first().row() >= mParameterTableModel->rowCount()-1)
       return;

    for(int i=0; i<idxSelection.size(); i++) {
        QModelIndex idx = idxSelection.at(i);
        mParameterTableModel->moveRows(QModelIndex(), idx.row(), 1,
                                    QModelIndex(), idx.row()+2);
    }

    disconnect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition);
    QItemSelection select;
    for(QModelIndex indx : qAsConst(idxSelection)) {
        QModelIndex leftIndex  = ui->gamsParameterTableView->model()->index(indx.row()+1, GamsParameterTableModel::COLUMN_OPTION_KEY);
        QModelIndex rightIndex = ui->gamsParameterTableView->model()->index(indx.row()+1, GamsParameterTableModel::COLUMN_ENTRY_NUMBER);
        QItemSelection rowSelection(leftIndex, rightIndex);
        select.merge(rowSelection, QItemSelectionModel::Select);
    }
    ui->gamsParameterTableView->selectionModel()->select(select, QItemSelectionModel::ClearAndSelect);
    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParameterEditor::setEditorExtended(bool extended)
{
    if (extended) {
        disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, this, &ParameterEditor::updateParameterTableModel );

        ui->gamsParameterTreeView->clearSelection();
        ui->gamsParameterTreeView->collapseAll();
        emit ParameterTableModelChanged(ui->gamsParameterCommandLine->currentText());
    } else  {
        connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged, this, &ParameterEditor::updateParameterTableModel, Qt::UniqueConnection );
    }
    mExtendedEditor->setVisible(extended);
    main->updateRunState();
    ui->gamsParameterCommandLine->setEnabled(!extended);
}

bool ParameterEditor::isEditorExtended()
{
    return mExtendedEditor->isVisible();
}

void ParameterEditor::on_newTableRowDropped(const QModelIndex &index)
{
    disconnect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition);
    ui->gamsParameterTableView->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );

    QString optionName = ui->gamsParameterTableView->model()->data(index, Qt::DisplayRole).toString();
    QModelIndexList definitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     optionName, 1);
    mOptionTokenizer->getOption()->setModified(optionName, true);
    for(QModelIndex item : qAsConst(definitionItems)) {
        ui->gamsParameterTreeView->model()->setData(item, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    if (mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumStr &&
        mOptionTokenizer->getOption()->getOptionType(optionName) != optTypeEnumInt &&
        mOptionTokenizer->getOption()->getOptionSubType(optionName) != optsubNoValue)
        ui->gamsParameterTableView->edit( mParameterTableModel->index(index.row(), GamsParameterTableModel::COLUMN_OPTION_VALUE) );

    showParameterDefinition();
    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParameterEditor::on_parameterTableNameChanged(const QString &from, const QString &to)
{
    if (QString::compare(from, to, Qt::CaseInsensitive)==0)
        return;

    disconnect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition);
    QModelIndexList fromDefinitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     from, 1);
    if (fromDefinitionItems.size() <= 0) {
        fromDefinitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         from, 1);
    }
    for(QModelIndex item : qAsConst(fromDefinitionItems)) {
        QModelIndex index = ui->gamsParameterTreeView->model()->index(item.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->gamsParameterTreeView->model()->setData(index, Qt::CheckState(Qt::Unchecked), Qt::CheckStateRole);
    }

    QModelIndexList toDefinitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     to, 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         to, 1);
    }
    for(QModelIndex item : qAsConst(toDefinitionItems)) {
        QModelIndex index = ui->gamsParameterTreeView->model()->index(item.row(), OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->gamsParameterTreeView->model()->setData(index, Qt::CheckState(Qt::Checked), Qt::CheckStateRole);
    }

    ui->gamsParameterTreeView->selectionModel()->clearSelection();
    if (toDefinitionItems.size() > 0) {
        ui->gamsParameterTreeView->selectionModel()->select(
                    QItemSelection (
                        ui->gamsParameterTreeView->model ()->index (toDefinitionItems.first().row() , 0),
                        ui->gamsParameterTreeView->model ()->index (toDefinitionItems.first().row(), ui->gamsParameterTreeView->model ()->columnCount () - 1)),
                    QItemSelectionModel::Select);
        ui->gamsParameterTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParameterEditor::on_parameterValueChanged(const QModelIndex &index)
{
    disconnect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition);
    ui->gamsParameterTreeView->selectionModel()->clearSelection();

    QModelIndex idx = index.sibling(index.row(), GamsParameterTableModel::COLUMN_OPTION_KEY);
    QString data = ui->gamsParameterTableView->model()->data(idx, Qt::DisplayRole).toString();
    QModelIndexList toDefinitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_OPTION_NAME),
                                                                     Qt::DisplayRole,
                                                                     data, 1);
    if (toDefinitionItems.size() <= 0) {
        toDefinitionItems = ui->gamsParameterTreeView->model()->match(ui->gamsParameterTreeView->model()->index(0, OptionDefinitionModel::COLUMN_SYNONYM),
                                                                         Qt::DisplayRole,
                                                                         data, 1);
    }

    if (toDefinitionItems.size() > 0) {
        ui->gamsParameterTreeView->selectionModel()->select(
                    QItemSelection (
                        ui->gamsParameterTreeView->model ()->index (toDefinitionItems.first().row() , 0),
                        ui->gamsParameterTreeView->model ()->index (toDefinitionItems.first().row(), ui->gamsParameterTreeView->model ()->columnCount () - 1)),
                    QItemSelectionModel::Select);
        ui->gamsParameterTreeView->scrollTo(toDefinitionItems.first(), QAbstractItemView::EnsureVisible);
    }
    connect(ui->gamsParameterTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ParameterEditor::findAndSelectionParameterFromDefinition, Qt::UniqueConnection);
}

void ParameterEditor::on_parameterTableModelChanged(const QString &commandLineStr)
{
    disconnect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            this, &ParameterEditor::updateParameterTableModel );
    disconnect(mParameterTableModel, &GamsParameterTableModel::optionModelChanged, this, static_cast<void(ParameterEditor::*)(const QList<OptionItem> &)> (&ParameterEditor::updateCommandLineStr));
    disconnect(this, static_cast<void(ParameterEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&ParameterEditor::commandLineChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit);

    mParameterTableModel->on_ParameterTableModelChanged(commandLineStr);

    connect(ui->gamsParameterCommandLine, &CommandLine::commandLineChanged,
            this, &ParameterEditor::updateParameterTableModel, Qt::UniqueConnection);
    connect(mParameterTableModel, &GamsParameterTableModel::optionModelChanged, this, static_cast<void(ParameterEditor::*)(const QList<OptionItem> &)> (&ParameterEditor::updateCommandLineStr), Qt::UniqueConnection);
    connect(this, static_cast<void(ParameterEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&ParameterEditor::commandLineChanged), mOptionTokenizer, &OptionTokenizer::formatItemLineEdit, Qt::UniqueConnection);
}

void ParameterEditor::resizeColumnsToContents()
{
    if (ui->gamsParameterTableView->hasFocus()) {
        if (ui->gamsParameterTableView->model()->rowCount()<=0)
            return;
        ui->gamsParameterTableView->resizeColumnToContents(GamsParameterTableModel::COLUMN_OPTION_KEY);
        ui->gamsParameterTableView->resizeColumnToContents(GamsParameterTableModel::COLUMN_OPTION_VALUE);
    } else  if (ui->gamsParameterTreeView->hasFocus()) {
        ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_OPTION_NAME);
        ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_SYNONYM);
        ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_DEF_VALUE);
        ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_RANGE);
        ui->gamsParameterTreeView->resizeColumnToContents(OptionDefinitionModel::COLUMN_TYPE);
    }
}

void ParameterEditor::setRunsActionGroup(QAction *aRun, QAction *aRunGDX, QAction *aCompile, QAction *aCompileGDX,
                                         QAction *aRunNeos, QAction *aRunEngine)
{
    mHasSSL = QSslSocket::supportsSsl();
    actionRun = aRun;
    actionCompile = aCompile;
    actionRun_with_GDX_Creation = aRunGDX;
    actionCompile_with_GDX_Creation = aCompileGDX;
    actionRunNeos = aRunNeos;
    actionRunEngine = aRunEngine;

    QMenu* runMenu = new QMenu;
    runMenu->addAction(actionRun);
    runMenu->addAction(actionRun_with_GDX_Creation);
    runMenu->addSeparator();
    runMenu->addAction(actionCompile);
    runMenu->addAction(actionCompile_with_GDX_Creation);
    runMenu->addSeparator();
    runMenu->addAction(actionRunNeos);
    runMenu->addSeparator();
    runMenu->addAction(actionRunEngine);
    actionRun->setShortcutVisibleInContextMenu(true);
    actionRun_with_GDX_Creation->setShortcutVisibleInContextMenu(true);
    actionCompile->setShortcutVisibleInContextMenu(true);
    actionCompile_with_GDX_Creation->setShortcutVisibleInContextMenu(true);
    actionRunNeos->setShortcutVisibleInContextMenu(true);
    actionRunEngine->setShortcutVisibleInContextMenu(true);

    ui->gamsRunToolButton->setMenu(runMenu);
    ui->gamsRunToolButton->setDefaultAction(actionRun);
}

void ParameterEditor::setInterruptActionGroup(QAction *aInterrupt, QAction *aStop)
{
    actionInterrupt = aInterrupt;
    actionInterrupt->setShortcutVisibleInContextMenu(true);
    actionStop = aStop;
    actionStop->setShortcutVisibleInContextMenu(true);

    QMenu* interruptMenu = new QMenu();
    interruptMenu->addAction(actionInterrupt);
    interruptMenu->addAction(actionStop);

    ui->gamsInterruptToolButton->setMenu(interruptMenu);
    ui->gamsInterruptToolButton->setDefaultAction(actionInterrupt);
}

void ParameterEditor::setRunActionsEnabled(bool enable)
{
    actionRun->setEnabled(enable);
    actionRun_with_GDX_Creation->setEnabled(enable);
    actionCompile->setEnabled(enable);
    actionCompile_with_GDX_Creation->setEnabled(enable);
    actionRunNeos->setEnabled(enable && mHasSSL);
    actionRunEngine->setEnabled(enable);
    ui->gamsRunToolButton->menu()->setEnabled(enable);
}

void ParameterEditor::setInterruptActionsEnabled(bool enable)
{
    actionInterrupt->setEnabled(enable);
    actionStop->setEnabled(enable);
    ui->gamsInterruptToolButton->menu()->setEnabled(enable);
}

void ParameterEditor::addActions()
{
    QAction* insertParameterAction = mContextMenu.addAction(Theme::icon(":/%1/insert"), "Insert new parameter", [this]() { insertParameter(); });
    insertParameterAction->setObjectName("actionInsert_option");
    insertParameterAction->setShortcut( QKeySequence("Ctrl+Return") );
    insertParameterAction->setShortcutVisibleInContextMenu(true);
    insertParameterAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(insertParameterAction);

    QAction* deleteAction = mContextMenu.addAction(Theme::icon(":/%1/delete-all"), "Delete selection", [this]() { deleteParameter(); });
    deleteAction->setObjectName("actionDelete_option");
    deleteAction->setShortcut( QKeySequence("Ctrl+Delete") );
    deleteAction->setShortcutVisibleInContextMenu(true);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(deleteAction);

    QAction* deleteAllAction = mContextMenu.addAction(Theme::icon(":/%1/delete-all"), "Delete all parameters", [this]() { deleteAllParameters(); });
    deleteAllAction->setObjectName("actionDeleteAll_option");
    deleteAllAction->setShortcut( QKeySequence("Alt+Delete") );
    deleteAllAction->setShortcutVisibleInContextMenu(true);
    deleteAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(deleteAllAction);

    QAction* moveUpAction = mContextMenu.addAction(Theme::icon(":/%1/move-up"), "Move up", [this]() { moveParameterUp(); });
    moveUpAction->setObjectName("actionMoveUp_option");
    moveUpAction->setShortcut( QKeySequence("Ctrl+Up") );
    moveUpAction->setShortcutVisibleInContextMenu(true);
    moveUpAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(moveUpAction);

    QAction* moveDownAction = mContextMenu.addAction(Theme::icon(":/%1/move-down"), "Move down", [this]() { moveParameterDown(); });
    moveDownAction->setObjectName("actionMoveDown_option");
    moveDownAction->setShortcut( QKeySequence("Ctrl+Down") );
    moveDownAction->setShortcutVisibleInContextMenu(true);
    moveDownAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(moveDownAction);

    QAction* showDefinitionAction = mContextMenu.addAction("Show parameter definition", [this]() { showParameterDefinition(); });
    showDefinitionAction->setObjectName("actionShowDefinition_option");
    showDefinitionAction->setShortcut( QKeySequence("Ctrl+F1") );
    showDefinitionAction->setShortcutVisibleInContextMenu(true);
    showDefinitionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(showDefinitionAction);

    QAction* showDuplicationAction = mContextMenu.addAction("Show all parameters of the same definition", [this]() { showParameterRecurrence(); });
    showDuplicationAction->setObjectName("actionShowRecurrence_option");
    showDuplicationAction->setShortcut( QKeySequence("Shift+F1") );
    showDuplicationAction->setShortcutVisibleInContextMenu(true);
    showDuplicationAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(showDuplicationAction);

    QAction* addThisOptionAction = mContextMenu.addAction(Theme::icon(":/%1/plus"), "Add this parameter", [this]() {
        QModelIndexList selection = ui->gamsParameterTreeView->selectionModel()->selectedRows();
        if (selection.size()>0)
            addParameterFromDefinition(selection.at(0));
    });
    addThisOptionAction->setObjectName("actionAddThisOption");
    addThisOptionAction->setShortcut( QKeySequence(Qt::Key_Return) );
    addThisOptionAction->setShortcutVisibleInContextMenu(true);
    addThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTreeView->addAction(addThisOptionAction);

    QAction* deleteThisOptionAction = mContextMenu.addAction(Theme::icon(":/%1/delete-all"), "Remove this parameter", [this]() {
        findAndSelectionParameterFromDefinition();
        deleteParameter();
    });
    deleteThisOptionAction->setObjectName("actionDeleteThisOption");
    deleteThisOptionAction->setShortcut( QKeySequence(Qt::Key_Delete) );
    deleteThisOptionAction->setShortcutVisibleInContextMenu(true);
    deleteThisOptionAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTreeView->addAction(deleteThisOptionAction);

    QAction* resizeColumns = mContextMenu.addAction("Resize columns to contents", [this]() { resizeColumnsToContents(); });
    resizeColumns->setObjectName("actionResize_columns");
    resizeColumns->setShortcut( QKeySequence("Ctrl+R") );
    resizeColumns->setShortcutVisibleInContextMenu(true);
    resizeColumns->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->gamsParameterTableView->addAction(resizeColumns);
    ui->gamsParameterTreeView->addAction(resizeColumns);
}

QList<int> ParameterEditor::getRecurrentParameter(const QModelIndex &index)
{
    QList<int> optionList;
    if (!mExtendedEditor->isVisible())
        return optionList;

    QString optionId = ui->gamsParameterTableView->model()->data( index.sibling(index.row(), GamsParameterTableModel::COLUMN_ENTRY_NUMBER), Qt::DisplayRole).toString();
    QModelIndexList indices = ui->gamsParameterTableView->model()->match(ui->gamsParameterTableView->model()->index(0, GamsParameterTableModel::COLUMN_ENTRY_NUMBER),
                                                                      Qt::DisplayRole,
                                                                      optionId, -1);

    for(QModelIndex idx : qAsConst(indices)) {
        if (idx.row() == index.row())
            continue;
        else
            optionList << idx.row();
    }
    return optionList;
}

QString ParameterEditor::getParameterTableEntry(int row)
{
    QModelIndex keyIndex = ui->gamsParameterTableView->model()->index(row, GamsParameterTableModel::COLUMN_OPTION_KEY);
    QVariant optionKey = ui->gamsParameterTableView->model()->data(keyIndex, Qt::DisplayRole);
    QModelIndex valueIndex = ui->gamsParameterTableView->model()->index(row, GamsParameterTableModel::COLUMN_OPTION_VALUE);
    QVariant optionValue = ui->gamsParameterTableView->model()->data(valueIndex, Qt::DisplayRole);
    return QString("%1%2%3").arg(optionKey.toString(),
                                 mOptionTokenizer->getOption()->getDefaultSeparator(),
                                 optionValue.toString());
}

QDockWidget* ParameterEditor::extendedEditor() const
{
    return mExtendedEditor;
}

OptionTokenizer *ParameterEditor::getOptionTokenizer() const
{
    return mOptionTokenizer;
}

bool ParameterEditor::isAParameterEditorFocused(QWidget *focusWidget) const
{
    return (focusWidget==ui->gamsParameterTableView || focusWidget==ui->gamsParameterTreeView || focusWidget==ui->gamsParameterCommandLine);
}

bool ParameterEditor::isAParameterTableFocused(QWidget *focusWidget) const
{
      return (focusWidget==ui->gamsParameterTableView);
}

QString ParameterEditor::getSelectedParameterName(QWidget *widget) const
{
    if (widget == ui->gamsParameterTableView) {
        QModelIndexList selection = ui->gamsParameterTableView->selectionModel()->selectedIndexes();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QVariant headerData = ui->gamsParameterTableView->model()->headerData(index.row(), Qt::Vertical, Qt::CheckStateRole);
            if (Qt::CheckState(headerData.toUInt())==Qt::PartiallyChecked) {
                return "";
            }
            QVariant data = ui->gamsParameterTableView->model()->data( index.sibling(index.row(),0) );
            if (mOptionTokenizer->getOption()->isValid(data.toString()))
               return data.toString();
            else if (mOptionTokenizer->getOption()->isASynonym(data.toString()))
                    return mOptionTokenizer->getOption()->getNameFromSynonym(data.toString());
            else
               return "";
        }
    } else if (widget == ui->gamsParameterTreeView) {
        QModelIndexList selection = ui->gamsParameterTreeView->selectionModel()->selectedRows();
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            QModelIndex  parentIndex =  ui->gamsParameterTreeView->model()->parent(index);
            if (parentIndex.row() >= 0) {
                return ui->gamsParameterTreeView->model()->data( parentIndex.sibling(parentIndex.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            } else {
                return ui->gamsParameterTreeView->model()->data( index.sibling(index.row(), OptionDefinitionModel::COLUMN_OPTION_NAME) ).toString();
            }
        }
    }
    return "";
}

QString ParameterEditor::getCurrentCommandLineData() const
{
    return ui->gamsParameterCommandLine->getParameterString();
}

void ParameterEditor::focus()
{
    if (isEditorExtended())
        if (ui->gamsParameterTableView->hasFocus())
            ui->gamsParameterSearch->setFocus(Qt::ShortcutFocusReason);
        else if (ui->gamsParameterSearch->hasFocus())
                ui->gamsParameterTreeView->setFocus(Qt::ShortcutFocusReason);
        else
            ui->gamsParameterTableView->setFocus(Qt::TabFocusReason);
    else
        ui->gamsParameterCommandLine->setFocus(Qt::ShortcutFocusReason);
}

}
}
}
