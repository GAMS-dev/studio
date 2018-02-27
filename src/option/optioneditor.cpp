#include "optioneditor.h"
#include "optioncompleterdelegate.h"
#include "optiondefinitionmodel.h"
#include "optionparametermodel.h"
#include "addoptionheaderview.h"

namespace gams {
namespace studio {

OptionEditor::OptionEditor(CommandLineOption* option, CommandLineTokenizer* tokenizer, QWidget *parent) :
    QWidget(parent), mCommandLineOption(option), mTokenizer(tokenizer)
{
    setupUi(this);
}

OptionEditor::~OptionEditor()
{

}

void OptionEditor::setupUi(QWidget* optionEditor)
{
    QList<OptionItem> optionItem = mTokenizer->tokenize(mCommandLineOption->lineEdit()->text());
    QString normalizedText = mTokenizer->normalize(optionItem);
    optionParamModel = new OptionParameterModel(normalizedText, mTokenizer,  this);

    if (optionEditor->objectName().isEmpty())
        optionEditor->setObjectName(QStringLiteral("OptionEditor"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(optionEditor->sizePolicy().hasHeightForWidth());
    optionEditor->setSizePolicy(sizePolicy);

    verticalLayout = new QVBoxLayout(optionEditor);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

    splitter = new QSplitter(optionEditor);
    splitter->setObjectName(QStringLiteral("splitter"));
    sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
    splitter->setSizePolicy(sizePolicy);
    splitter->setOrientation(Qt::Horizontal);
    commandLineTableView = new QTableView(splitter);
    commandLineTableView->setObjectName(QStringLiteral("commandLineTableView"));
    commandLineTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    commandLineTableView->setItemDelegate( new OptionCompleterDelegate(mTokenizer, commandLineTableView));
    commandLineTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    commandLineTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    commandLineTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    commandLineTableView->setAutoScroll(true);
    commandLineTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    commandLineTableView->setModel( optionParamModel );
    commandLineTableView->horizontalHeader()->setStretchLastSection(true);

    AddOptionHeaderView* headerView = new AddOptionHeaderView(Qt::Horizontal, commandLineTableView);
    headerView->setSectionResizeMode(QHeaderView::Stretch);
    commandLineTableView->setHorizontalHeader(headerView);

    splitter->addWidget(commandLineTableView);
    splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));

    verticalLayoutWidget = new QWidget(splitter);
    verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
    optionDefinition_VLayout = new QVBoxLayout(verticalLayoutWidget);
    optionDefinition_VLayout->setObjectName(QStringLiteral("optionDefinition_VLayout"));
    optionDefinition_VLayout->setContentsMargins(0, 0, 0, 0);
    searchLineEdit = new QLineEdit(verticalLayoutWidget);
    searchLineEdit->setObjectName(QStringLiteral("searchLineEdit"));
    searchLineEdit->setPlaceholderText("Search Option...");

    optionDefinition_VLayout->addWidget(searchLineEdit);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    OptionDefinitionModel* optdefmodel =  new OptionDefinitionModel(mTokenizer->getGamsOption(), this);
    proxymodel->setFilterKeyColumn(-1);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);

    optionDefinitionTreeView = new QTreeView(verticalLayoutWidget);
    optionDefinitionTreeView->setObjectName(QStringLiteral("optionDefinitionTreeView"));
    optionDefinitionTreeView->setItemsExpandable(true);
    optionDefinitionTreeView->setSortingEnabled(true);
    optionDefinitionTreeView->sortByColumn(0, Qt::AscendingOrder);
    optionDefinitionTreeView->setModel( proxymodel );
    optionDefinitionTreeView->resizeColumnToContents(0);
    optionDefinitionTreeView->resizeColumnToContents(2);
    optionDefinitionTreeView->resizeColumnToContents(3);
    optionDefinitionTreeView->setAlternatingRowColors(true);

    optionDefinition_VLayout->addWidget(optionDefinitionTreeView);

    splitter->addWidget(verticalLayoutWidget);

    verticalLayout->addWidget(splitter);

    // retranslateUi
    optionEditor->setWindowTitle(QApplication::translate("optionEditor", "Editor", nullptr));
    searchLineEdit->setText(QString());

    updateCommandLineStr( normalizedText );

    connect(searchLineEdit, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));
//    connect(commandLineTableView->verticalHeader(), &QHeaderView::sectionClicked,
//            optionParamModel, &OptionParameterModel::toggleActiveOptionItem);
    connect(commandLineTableView, &QTableView::customContextMenuRequested,this, &OptionEditor::showOptionContextMenu);

    connect(this, &OptionEditor::optionTableModelChanged, optionParamModel, &OptionParameterModel::updateCurrentOption);

    connect(optionParamModel, &OptionParameterModel::optionModelChanged,
            this, static_cast<void(OptionEditor::*)(const QList<OptionItem> &)> (&OptionEditor::updateCommandLineStr));
    connect(this, static_cast<void(OptionEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionEditor::commandLineOptionChanged),
            mTokenizer, &CommandLineTokenizer::formatItemLineEdit);
}

QList<OptionItem> OptionEditor::getCurrentListOfOptionItems()
{
    return optionParamModel->getCurrentListOfOptionItems();
}

void OptionEditor::updateTableModel(QLineEdit* lineEdit, const QString &commandLineStr)
{
     emit optionTableModelChanged(commandLineStr);
}

void OptionEditor::updateCommandLineStr(const QString &commandLineStr)
{
    if (isHidden())
       return;

    mCommandLineOption->lineEdit()->setText( commandLineStr );
    emit commandLineOptionChanged(mCommandLineOption->lineEdit(), commandLineStr);
}

void OptionEditor::updateCommandLineStr(const QList<OptionItem> &opionItems)
{
    if (isHidden())
       return;

    emit commandLineOptionChanged(mCommandLineOption->lineEdit(), opionItems);
}

void OptionEditor::showOptionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = commandLineTableView->selectionModel()->selectedRows();

    QMenu menu(this);
    QIcon addIcon(":/img/plus");
    QAction* addAction = menu.addAction(addIcon, "add new option");
    QAction* insertAction = menu.addAction("insert new option");
    menu.addSeparator();
    QIcon moveUpIcon(":/img/move-up");
    QAction* moveUpAction = menu.addAction(moveUpIcon, "move selected option up");
    QIcon moveDownIcon(":/img/move-down");
    QAction* moveDownAction = menu.addAction(moveDownIcon, "move selected option down");
    menu.addSeparator();
    QIcon deleteIcon(":/img/minus");
    QAction* deleteAction = menu.addAction(deleteIcon, "remove selected option");
    menu.addSeparator();
    QIcon deleteAllIcon(":/img/delete-all");
    QAction* deleteAllActions = menu.addAction(deleteAllIcon, "remove all options");

    if (commandLineTableView->model()->rowCount() <= 0) {
        deleteAllActions->setVisible(false);
    }
    if (selection.count() <= 0) {
        insertAction->setVisible(false);
        deleteAction->setVisible(false);
        moveUpAction->setVisible(false);
        moveDownAction->setVisible(false);
    } else {
        QModelIndex index = selection.at(0);
        if (index.row()==0)
            moveUpAction->setVisible(false);
        if (index.row()+1 == commandLineTableView->model()->rowCount())
            moveDownAction->setVisible(false);
    }

    QAction* action = menu.exec(commandLineTableView->viewport()->mapToGlobal(pos));
    if (action == addAction) {
         commandLineTableView->model()->insertRows(commandLineTableView->model()->rowCount(), 1, QModelIndex());
    } else if (action == insertAction) {
            if (selection.count() > 0) {
                QModelIndex index = selection.at(0);
                commandLineTableView->model()->insertRows(index.row(), 1, QModelIndex());
            }
    } else if (action == moveUpAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            commandLineTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()-1);
        }

    } else if (action == moveDownAction) {
        if (selection.count() > 0) {
            QModelIndex index = selection.at(0);
            commandLineTableView->model()->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), index.row()+2);
        }
    } else if (action == deleteAction) {
             if (selection.count() > 0) {
                 QModelIndex index = selection.at(0);
                 commandLineTableView->model()->removeRow(index.row(), QModelIndex());
             }
    } else if (action == deleteAllActions) {
        emit optionTableModelChanged("");
    }
}

} // namespace studio
} // namespace gams
