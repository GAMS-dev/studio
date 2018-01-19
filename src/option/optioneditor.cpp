#include "optioneditor.h"
#include "optionconfigurator.h"
#include "optioncompleterdelegate.h"
#include "optiondefinitionmodel.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionEditor::OptionEditor(CommandLineOption* option, CommandLineTokenizer* tokenizer, QWidget *parent) :
    mCommandLineOption(option), QWidget(parent)
{
    QList<OptionItem> optionItem = tokenizer->tokenize(mCommandLineOption->lineEdit()->text());
    QString normalizedText = tokenizer->normalize(optionItem);
    OptionParameterModel* optionParamModel = new OptionParameterModel(normalizedText, tokenizer,  this);

    setupUi(this);
    updateCommandLineStr( normalizedText );

    commandLineTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    commandLineTableView->setItemDelegate( new OptionCompleterDelegate(tokenizer, commandLineTableView));
    commandLineTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    commandLineTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    commandLineTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    commandLineTableView->setAutoScroll(true);
    commandLineTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    commandLineTableView->setModel( optionParamModel );
    commandLineTableView->horizontalHeader()->setStretchLastSection(true);
    commandLineTableView->horizontalHeader()->setAccessibleDescription("Active/Deactivate the option when run");
    commandLineTableView->resizeColumnsToContents();
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,2);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    OptionDefinitionModel* optdefmodel =  new OptionDefinitionModel(tokenizer->getGamsOption(), this);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);

    optionDefinitionTreeView->setItemsExpandable(true);
    optionDefinitionTreeView->setSortingEnabled(true);
    optionDefinitionTreeView->sortByColumn(0, Qt::AscendingOrder);
    optionDefinitionTreeView->setModel( proxymodel );
    optionDefinitionTreeView->resizeColumnToContents(0);
    optionDefinitionTreeView->resizeColumnToContents(2);
    optionDefinitionTreeView->resizeColumnToContents(3);
    optionDefinitionTreeView->setAlternatingRowColors(true);

    searchLineEdit->setPlaceholderText("Search Option...");

//    connect(searchLineEdit, &QLineEdit::textChanged,
//            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));
//    connect(commandLineTableView->verticalHeader(), &QHeaderView::sectionClicked,
//            optionParamModel, &OptionParameterModel::toggleActiveOptionItem);
//    connect(commandLineTableView, &QTableView::customContextMenuRequested,
//            this, &OptionEditor::showOptionContextMenu);
//    connect(optionParamModel, &OptionParameterModel::optionModelChanged,
//            this, static_cast<void(OptionEditor::*)(const QList<OptionItem> &)> (&OptionEditor::updateCommandLineStr));
//    connect(this, static_cast<void(OptionEditor::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionEditor::commandLineOptionChanged),
//            tokenizer, &CommandLineTokenizer::formatItemLineEdit);

//    connect(mCommandLineOption, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
//            optionParamModel, &OptionParameterModel::updateCurrentOption );
//    connect(mCommandLineOption, &QComboBox::editTextChanged,
//            optionParamModel, &OptionParameterModel::validateChangedOption );
}

OptionEditor::~OptionEditor()
{

}

void OptionEditor::setupUi(QWidget* optionEditor)
{
    if (optionEditor->objectName().isEmpty())
        optionEditor->setObjectName(QStringLiteral("OptionEditor"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    splitter->addWidget(commandLineTableView);
    verticalLayoutWidget = new QWidget(splitter);
    verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
    optionDefinition_VLayout = new QVBoxLayout(verticalLayoutWidget);
    optionDefinition_VLayout->setObjectName(QStringLiteral("optionDefinition_VLayout"));
    optionDefinition_VLayout->setContentsMargins(0, 0, 0, 0);
    searchLineEdit = new QLineEdit(verticalLayoutWidget);
    searchLineEdit->setObjectName(QStringLiteral("searchLineEdit"));

    optionDefinition_VLayout->addWidget(searchLineEdit);

    optionDefinitionTreeView = new QTreeView(verticalLayoutWidget);
    optionDefinitionTreeView->setObjectName(QStringLiteral("optionDefinitionTreeView"));

    optionDefinition_VLayout->addWidget(optionDefinitionTreeView);

    splitter->addWidget(verticalLayoutWidget);

    verticalLayout->addWidget(splitter);

    // retranslateUi
    optionEditor->setWindowTitle(QApplication::translate("optionEditor", "Editor", nullptr));
    searchLineEdit->setText(QString());
}

void OptionEditor::updateCommandLineStr(const QString &commandLineStr)
{
    mCommandLineOption->lineEdit()->setText( commandLineStr );
    emit commandLineOptionChanged(mCommandLineOption->lineEdit(), commandLineStr);
}

void OptionEditor::updateCommandLineStr(const QList<OptionItem> &opionItems)
{
    emit commandLineOptionChanged(mCommandLineOption->lineEdit(), opionItems);
}

void OptionEditor::showOptionContextMenu(const QPoint &pos)
{
    QModelIndexList selection = commandLineTableView->selectionModel()->selectedRows();

    QMenu menu(this);
    QAction* addAction = menu.addAction("add new option");
    QAction* insertAction = menu.addAction("insert new option");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction("delete selected option");

    if (selection.count() <= 0) {
        insertAction->setVisible(false);
        deleteAction->setVisible(false);
    }

    QAction* action = menu.exec(commandLineTableView->viewport()->mapToGlobal(pos));
    if (action == addAction) {
         commandLineTableView->model()->insertRows(commandLineTableView->model()->rowCount(), 1, QModelIndex());
    } else if (action == insertAction) {
            if (selection.count() > 0) {
                QModelIndex index = selection.at(0);
                commandLineTableView->model()->insertRows(index.row(), 1, QModelIndex());
            }
   } else if (action == deleteAction) {
             if (selection.count() > 0) {
                 QModelIndex index = selection.at(0);
                 commandLineTableView->model()->removeRow(index.row(), QModelIndex());
             }
    }
}

} // namespace studio
} // namespace gams
