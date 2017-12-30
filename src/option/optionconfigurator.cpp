#include "optionconfigurator.h"
#include "optioncompleterdelegate.h"
#include "optiondefinitionmodel.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionConfigurator::OptionConfigurator(const QString& label, const QString& lineEditText, CommandLineTokenizer* tokenizer, QWidget *parent):
     QFrame(parent)
{
    connect(this, &OptionConfigurator::commandLineOptionChanged,
            tokenizer, &CommandLineTokenizer::formatLineEditTextFormat);

    QList<OptionItem> optionItem = tokenizer->tokenize(lineEditText);
    QString normalizedText = tokenizer->normalize(optionItem);
    OptionParameterModel* optionParamModel = new OptionParameterModel(normalizedText, tokenizer,  this);

    ui.setupUi(this);
    ui.fileLabel->setText( label );
    updateCommandLineStr( normalizedText );
//    ui.commandLineEdit->setReadOnly( true );
    ui.commandLineEdit->setClearButtonEnabled(true);

    ui.showOptionDefintionCheckBox->setChecked(false);
    ui.splitter->widget(1)->hide();

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

    OptionDefinitionModel* optdefmodel =  new OptionDefinitionModel(tokenizer->getGamsOption(), this);
    ui.optionDefintionTreeView->setItemsExpandable(true);
    ui.optionDefintionTreeView->setModel( optdefmodel );

    connect(ui.showOptionDefintionCheckBox, &QCheckBox::clicked, this, &OptionConfigurator::toggleOptionDefinition);
    connect(ui.commandLineTableView->verticalHeader(), &QHeaderView::sectionClicked,
            this, &OptionConfigurator::toggleActiveOptionItem);
    connect(optionParamModel, &OptionParameterModel::editCompleted,
            this, &OptionConfigurator::updateCommandLineStr);
}

OptionConfigurator::~OptionConfigurator()
{
}

void OptionConfigurator::toggleActiveOptionItem(int index)
{
    QAbstractItemModel* model = ui.commandLineTableView->model();
    model->setHeaderData( index,
                          Qt::Vertical,
                          Qt::CheckState(model->headerData(index, Qt::Vertical, Qt::CheckStateRole).toUInt()) != Qt::Checked ? Qt::Checked : Qt::Unchecked,
                          Qt::CheckStateRole );
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

} // namespace studio
} // namespace gams
