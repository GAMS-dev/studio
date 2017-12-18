#include "optionconfigurator.h"

namespace gams {
namespace studio {

OptionConfigurator::OptionConfigurator(QString label, QLineEdit* lineEditText, QWidget *parent):
     QFrame(parent)
{
    ui.setupUi(this);
    ui.fileLabel->setText( label );
    ui.commandLineEdit->setText( lineEditText->text() );
    ui.commandLineEdit->setReadOnly( true );
    ui.showOptionDefintionCheckBox->setChecked(true);
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
