#include "optionconfigurator.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionConfigurator::OptionConfigurator(const QString& label, const QString& lineEditText, CommandLineTokenizer* tokenizer, QWidget *parent):
     QFrame(parent)
{
    ui.setupUi(this);
    ui.fileLabel->setText( label );
    ui.commandLineEdit->setText( lineEditText );
    ui.commandLineEdit->setReadOnly( true );
    ui.showOptionDefintionCheckBox->setChecked(true);

    ui.commandLineTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.commandLineTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.commandLineTableView->setAutoScroll(true);
    ui.commandLineTableView->setModel( new OptionParameterModel(lineEditText, tokenizer,  this) );

    connect(ui.showOptionDefintionCheckBox, &QCheckBox::clicked, this, &OptionConfigurator::toggleOptionDefinition);
}

OptionConfigurator::~OptionConfigurator()
{
}

void OptionConfigurator::toggleOptionDefinition(bool checked)
{
    if (checked)
        ui.splitter->widget(0)->show();
    else
        ui.splitter->widget(0)->hide();
}

} // namespace studio
} // namespace gams
