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

    if (validateFlag)  {
       QString cmlStr = "=a a c gdx=default=s.g a = c --limcow 2 =abc /o ouut.out";
       QList<OptionItem> list = mCommandLineTokenizer->tokenize(cmlStr);
       qDebug() << cmlStr;
       for(OptionItem item : list) {
           qDebug() << QString("[%1, %2] = (%3, %4)=>['%5', '%6']").arg(item.key).arg(item.value).arg(item.keyPosition).arg(item.valuePosition)
                    .arg(cmlStr.mid(item.keyPosition, item.key.size()))
                    .arg(cmlStr.mid(item.valuePosition, item.value.size()));
       }
    }

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
    // TODO: validate option key and value against optgams.def
    mCurrentOption = text.simplified();

    if (mCurrentOption.isEmpty())
        return;

    if (mValidated)  {
        clearLineEditTextFormat(this->lineEdit());

        QList<OptionItem> list = mCommandLineTokenizer->tokenize(mCurrentOption);
        qDebug() << mCurrentOption;
        for(OptionItem item : list) {
           qDebug() << QString("[%1, %2] = (%3, %4)=>['%5', '%6']").arg(item.key).arg(item.value).arg(item.keyPosition).arg(item.valuePosition)
                    .arg(mCurrentOption.mid(item.keyPosition, item.key.size()))
                    .arg(mCurrentOption.mid(item.valuePosition, item.value.size()));
        }
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
    qDebug() << QString("err.size()=%1").arg(errList.size());


    QString errorMessage;
    QList<QInputMethodEvent::Attribute> attributes;
    foreach(const OptionError err, errList)   {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = err.formatRange.start - lineEdit->cursorPosition();
        int length = err.formatRange.length;
        QVariant value = err.formatRange.format;
        qDebug() << QString("   start=%1, length=%2").arg(start).arg(length);
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));

        if (errorMessage.isEmpty()) {
            errorMessage.append("Error: Parameter error(s) or deprecation");
        }
        if (err.type == InvalidKey) {
            errorMessage.append("\n   "+ lineEdit->text().mid(err.formatRange.start, err.formatRange.length)+" (Unknown option)");
        } else if (err.type == InvalidValue) {
            errorMessage.append("\n   "+ lineEdit->text().mid(err.formatRange.start, err.formatRange.length)+" (Invalid value)");
        } else if (err.type == Deprecated) {
            errorMessage.append("\n   "+ lineEdit->text().mid(err.formatRange.start, err.formatRange.length)+" (Deprecated, will be ignored)");
        } else if (err.type == NoKey) {
            errorMessage.append("\n   "+ lineEdit->text().mid(err.formatRange.start, err.formatRange.length)+" (Option keyword expected)");
        }
    }
    if (!errorMessage.isEmpty())
        lineEdit->setToolTip(errorMessage);
    else
        lineEdit->setToolTip("");

    qDebug() << QString("   errorMessage=[%1]").arg(errorMessage);
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
