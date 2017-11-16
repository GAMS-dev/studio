#include "commandlineoption.h"

CommandLineOption::CommandLineOption(QWidget* parent) : QComboBox(parent)
{
    this->setEditable(true);
    this->showPopup();
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &CommandLineOption::activateRunWithCurrentOption );
    connect(this, &QComboBox::editTextChanged,
            this, &CommandLineOption::validateChangedOption );
}

void CommandLineOption::activateRunWithCurrentOption(QString text)
{
    mCurrentOption = text;
    emit runWithChangedOption(text);
}

void CommandLineOption::validateChangedOption(QString text)
{
    mCurrentOption = text;
}

QString CommandLineOption::getCurrentOption() const
{
    return mCurrentOption;
}
