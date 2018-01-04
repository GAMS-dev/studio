#include "optionconfigurator.h"
#include "optioncompleterdelegate.h"
#include "optiondefinitionmodel.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionConfigurator::OptionConfigurator(const QString& label, const QString& lineEditText, CommandLineTokenizer* tokenizer, QWidget *parent):
     QFrame(parent)
{
    connect(this, static_cast<void(OptionConfigurator::*)(QLineEdit*, const QString &)>(&OptionConfigurator::commandLineOptionChanged),
            tokenizer, &CommandLineTokenizer::formatTextLineEdit);

    QList<OptionItem> optionItem = tokenizer->tokenize(lineEditText);
    QString normalizedText = tokenizer->normalize(optionItem);
    OptionParameterModel* optionParamModel = new OptionParameterModel(normalizedText, tokenizer,  this);

    ui.setupUi(this);
    ui.fileLabel->setText( label );
    updateCommandLineStr( normalizedText );
//    ui.commandLineEdit->setReadOnly( true );
    ui.commandLineEdit->setClearButtonEnabled(true);

    ui.showOptionDefintionCheckBox->setChecked(true);
//    ui.commandLineTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.commandLineTableView->setItemDelegate( new OptionCompleterDelegate(tokenizer, ui.commandLineTableView));
    ui.commandLineTableView->setEditTriggers(QAbstractItemView::DoubleClicked
                       | QAbstractItemView::EditKeyPressed
                       | QAbstractItemView::AnyKeyPressed );
    ui.commandLineTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.commandLineTableView->setAutoScroll(true);
    ui.commandLineTableView->setModel( optionParamModel );
    ui.commandLineTableView->horizontalHeader()->setStretchLastSection(true);
    ui.commandLineTableView->horizontalHeader()->setAccessibleDescription("Active/Deactivate the option when run");
    ui.commandLineTableView->resizeColumnsToContents();
    ui.splitter->setStretchFactor(0,1);
    ui.splitter->setStretchFactor(1,2);

    QSortFilterProxyModel* proxymodel = new OptionSortFilterProxyModel(this);
    OptionDefinitionModel* optdefmodel =  new OptionDefinitionModel(tokenizer->getGamsOption(), this);
    proxymodel->setSourceModel( optdefmodel );
    proxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);

    ui.optionDefinitionTreeView->setItemsExpandable(true);
    ui.optionDefinitionTreeView->setSortingEnabled(true);
    ui.optionDefinitionTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui.optionDefinitionTreeView->setModel( proxymodel );
    ui.optionDefinitionTreeView->resizeColumnToContents(0);
    ui.optionDefinitionTreeView->resizeColumnToContents(2);
    ui.optionDefinitionTreeView->resizeColumnToContents(3);
    ui.optionDefinitionTreeView->setAlternatingRowColors(true);

    ui.searchLineEdit->setPlaceholderText("Search Option...");
    connect(ui.searchLineEdit, &QLineEdit::textChanged,
            proxymodel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));
    connect(ui.showOptionDefintionCheckBox, &QCheckBox::clicked, this, &OptionConfigurator::toggleOptionDefinition);
    connect(ui.commandLineTableView->verticalHeader(), &QHeaderView::sectionClicked,
            optionParamModel, &OptionParameterModel::toggleActiveOptionItem);
    connect(optionParamModel, &OptionParameterModel::optionModelChanged,
            this, static_cast<void(OptionConfigurator::*)(const QList<OptionItem> &)> (&OptionConfigurator::updateCommandLineStr));
    connect(this, static_cast<void(OptionConfigurator::*)(QLineEdit*, const QList<OptionItem> &)>(&OptionConfigurator::commandLineOptionChanged),
            tokenizer, &CommandLineTokenizer::formatItemLineEdit);
}

OptionConfigurator::~OptionConfigurator()
{
}

void OptionConfigurator::toggleOptionDefinition(bool checked)
{
    if (checked)
        ui.splitter->widget(1)->show();
    else
        ui.splitter->widget(1)->hide();
}

void OptionConfigurator::updateCommandLineStr(const QString &commandLineStr)
{
    ui.commandLineEdit->setText( commandLineStr );
    emit commandLineOptionChanged(ui.commandLineEdit, commandLineStr);
}

void OptionConfigurator::updateCommandLineStr(const QList<OptionItem> &optionItems)
{
    emit commandLineOptionChanged(ui.commandLineEdit, optionItems);
}

} // namespace studio
} // namespace gams
