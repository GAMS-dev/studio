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

    createOptionDefinitionWidget();
    connect(ui.showOptionDefintionCheckBox, &QCheckBox::clicked, this, &OptionConfigurator::toggleOptionDefinition);
}

OptionConfigurator::~OptionConfigurator()
{
    if (searchLineEdit)
        delete searchLineEdit;
    if (optionDefintionTreeView)
       delete optionDefintionTreeView;
}

void OptionConfigurator::toggleOptionDefinition(bool checked)
{
    if (checked) {
        createOptionDefinitionWidget();
    } else {
        if (ui.splitter->widget(0))
            ui.splitter->widget(0)->deleteLater();
    }
}

void OptionConfigurator::createOptionDefinitionWidget()
{
    QWidget* layoutWidget = new QWidget(ui.splitter);
    ui.optionDefinition_VLayout = new QVBoxLayout(layoutWidget);
    ui.optionDefinition_VLayout->setObjectName(QStringLiteral("optionDefinition_VLayout"));
    ui.optionDefinition_VLayout->setContentsMargins(0, 0, 0, 0);

    searchLineEdit = new QLineEdit(layoutWidget);
    searchLineEdit->setObjectName(QStringLiteral("searchLineEdit"));
    searchLineEdit->setText("Search...");
    ui.optionDefinition_VLayout->addWidget(searchLineEdit);

    optionDefintionTreeView = new QTreeView(layoutWidget);
    optionDefintionTreeView->setObjectName(QStringLiteral("optionDefintionTreeView"));
    optionDefintionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    optionDefintionTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui.optionDefinition_VLayout->addWidget(optionDefintionTreeView);

    ui.splitter->insertWidget(0, layoutWidget);
}

} // namespace studio
} // namespace gams
