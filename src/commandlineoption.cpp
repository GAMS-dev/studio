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

void CommandLineOption::updateCurrentOption(QString text)
{
    mCurrentOption = text.simplified();
}

void CommandLineOption::validateChangedOption(QString text)
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
       qDebug() << this->lineEdit()->text();
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

void CommandLineOption::clearLineEditTextFormat(QLineEdit *lineEdit)
{
    setLineEditTextFormat(lineEdit, QList<QTextLayout::FormatRange>());
}

void CommandLineOption::setLineEditTextFormat(QLineEdit *lineEdit, const QList<QTextLayout::FormatRange> &frList)
{
    qDebug() << QString("fr.size()=%1").arg(frList.size());

    QList<QInputMethodEvent::Attribute> attributes;
    foreach(const QTextLayout::FormatRange& fr, frList)   {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = fr.start - lineEdit->cursorPosition();
        int length = fr.length;
        QVariant value = fr.format;
        qDebug() << QString("   start=%1, length=%2").arg(start).arg(length);
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
    }
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
