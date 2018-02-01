#include <QKeyEvent>
#include "commandlineoption.h"

namespace gams {
namespace studio {

CommandLineOption::CommandLineOption(bool validateFlag, QWidget* parent) :
    QComboBox(parent), mValidated(validateFlag)
{
    this->setDisabled(true);
    this->setEditable(true);
    this->setCurrentIndex(-1);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setInsertPolicy(QComboBox::InsertAtTop);
    this->lineEdit()->setClearButtonEnabled(true);
    this->mCurrentContext = "";
    this->mCurrentOption = "";
    this->mCurrentIndex = -1;
}

CommandLineOption::~CommandLineOption()
{
}

void CommandLineOption::validateChangedOption(const QString &text)
{
    mCurrentOption = text.simplified();

    this->lineEdit()->setToolTip("");
//  also allow empty option to be validated
//    if (mCurrentOption.isEmpty())
//        return;

    if (mValidated)
       emit commandLineOptionChanged(this->lineEdit(), text);
}

QString CommandLineOption::getCurrentOption() const
{
    return mCurrentOption;
}

void CommandLineOption::keyPressEvent(QKeyEvent *event)
{
    QComboBox::keyPressEvent(event);
    if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
        emit optionRunChanged();
    }
}

QString CommandLineOption::getCurrentContext() const
{
    return mCurrentContext;
}

void CommandLineOption::setCurrentContext(const QString &currentContext)
{
    mCurrentContext = currentContext;
}

void CommandLineOption::resetCurrentValue()
{
    mCurrentContext = "";
    mCurrentOption = "";
}

bool CommandLineOption::isValidated() const
{
    return mValidated;
}

void CommandLineOption::validated(bool value)
{
    mValidated = value;
}

} // namespace studio
} // namespace gams
