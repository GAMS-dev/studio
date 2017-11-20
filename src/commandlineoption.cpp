#include <QKeyEvent>
#include "commandlineoption.h"

namespace gams {
namespace studio {

CommandLineOption::CommandLineOption(QWidget* parent) : QComboBox(parent)
{
    this->setEditable(true);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(this, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &CommandLineOption::updateCurrentOption );
    connect(this, &QComboBox::editTextChanged,
            this, &CommandLineOption::validateChangedOption );
}

void CommandLineOption::updateCurrentOption(QString text)
{
    mCurrentOption = text;
}

void CommandLineOption::validateChangedOption(QString text)
{
    // TODO: validate option key and value against optgams.def
    mCurrentOption = text;
}

QString CommandLineOption::getCurrentOption() const
{
    return mCurrentOption;
}

void CommandLineOption::keyPressEvent(QKeyEvent *event)
{
    QComboBox::keyPressEvent(event);
    if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
        emit runWithChangedOption(this->getCurrentOption());
    }
}

} // namespace studio
} // namespace gams
