#include "optionconfigurator.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionConfigurator::OptionConfigurator(const QString& label, const QString& lineEditText, CommandLineTokenizer* tokenizer, QWidget *parent):
     QFrame(parent)
{
    QList<OptionItem> optionItem = tokenizer->tokenize(lineEditText);
    QString normalizedText = tokenizer->normalize(optionItem);

    ui.setupUi(this);
    ui.fileLabel->setText( label );
    ui.commandLineEdit->setText( normalizedText );
    ui.commandLineEdit->setReadOnly( true );
    ui.showOptionDefintionCheckBox->setChecked(true);

    ui.commandLineTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.commandLineTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.commandLineTableView->setAutoScroll(true);
    ui.commandLineTableView->setModel( new OptionParameterModel(normalizedText, optionItem,  this) );
    ui.commandLineTableView->horizontalHeader()->setStretchLastSection(true);
    ui.commandLineTableView->horizontalHeader()->setAccessibleDescription("Active/Deactivate the option when run");
//    ui.commandLineTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    ui.commandLineTableView->resizeColumnsToContents();

    ui.splitter->setStretchFactor(0,1);
    ui.splitter->setStretchFactor(1,2);

    connect(ui.showOptionDefintionCheckBox, &QCheckBox::clicked, this, &OptionConfigurator::toggleOptionDefinition);
    connect(ui.commandLineTableView->verticalHeader(), &QHeaderView::sectionClicked,
            this, &OptionConfigurator::toggleActiveOptionItem);
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

} // namespace studio
} // namespace gams
