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
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setInsertPolicy(QComboBox::InsertAtTop);

    mCommandLineTokenizer = new CommandLineTokenizer;

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
        clearLineEditTextFormat(this->lineEdit());

        QList<OptionItem> list = mCommandLineTokenizer->tokenize(text);
        setLineEditTextFormat(this->lineEdit(), mCommandLineTokenizer->format(list));
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

void CommandLineOption::clearLineEditTextFormat(QLineEdit *lineEdit)
{
    setLineEditTextFormat(lineEdit, QList<OptionError>());
}

void CommandLineOption::setLineEditTextFormat(QLineEdit *lineEdit, const QList<OptionError> &errList)
{
    QString errorMessage = "";
    QList<QInputMethodEvent::Attribute> attributes;
    foreach(const OptionError err, errList)   {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = err.formatRange.start - lineEdit->cursorPosition();
        int length = err.formatRange.length;
        QVariant value = err.formatRange.format;
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));

        if (errorMessage.isEmpty()) {
            errorMessage.append("Error: Parameter error(s)");
        }
        errorMessage.append("\n    " + err.message);
    }
    if (!errorMessage.isEmpty())
        lineEdit->setToolTip(errorMessage);
    else
        lineEdit->setToolTip("");

    QInputMethodEvent event(QString(), attributes);
    QCoreApplication::sendEvent(lineEdit, &event);
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
