#include <QKeyEvent>
#include "commandlineoption.h"

namespace gams {
namespace studio {

CommandLineOption::CommandLineOption(bool validateFlag, CommandLineTokenizer* tokenizer, QWidget* parent) :
    QComboBox(parent), mValidated(validateFlag), mCommandLineTokenizer(tokenizer)
{
    this->setDisabled(true);
    this->setEditable(true);
    this->setCurrentIndex(-1);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setInsertPolicy(QComboBox::InsertAtTop);

    connect(this, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &CommandLineOption::updateCurrentOption );
    connect(this, &QComboBox::editTextChanged,
            this, &CommandLineOption::validateChangedOption );
}

CommandLineOption::~CommandLineOption()
{
   delete mCommandLineTokenizer;
}

void CommandLineOption::updateCurrentOption(const QString &text)
{
    mCurrentOption = text.simplified();
}

void CommandLineOption::validateChangedOption(const QString &text)
{
    mCurrentOption = text.simplified();

    this->lineEdit()->setToolTip("");
    if (mCurrentOption.isEmpty())
        return;

    if (mValidated)  {
        mCommandLineTokenizer->clearLineEditTextFormat(this->lineEdit());
        mCommandLineTokenizer->setLineEditTextFormat(this->lineEdit(), text);
    }
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
