#ifndef COMMANDLINEOPTION_H
#define COMMANDLINEOPTION_H

#include <QtCore>
#include<QComboBox>
#include "commandlinetokenizer.h"

namespace gams {
namespace studio {

class CommandLineOption : public QComboBox
{
    Q_OBJECT

public:
    CommandLineOption(bool validateFlag, CommandLineTokenizer* tokenizer, QWidget* parent);
    ~CommandLineOption();

    QString getCurrentOption() const;

    bool isValidated() const;
    void validated(bool value);

    QString getCurrentContext() const;
    void setCurrentContext(const QString &currentContext);

    void resetCurrentValue();

signals:
    void optionRunChanged();
    void optionRunWithParameterChanged(const QString &parameter);

public slots:
    void updateCurrentOption(const QString &text);
    void validateChangedOption(const QString &text);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    QString mCurrentOption;
    QString mCurrentContext;
    bool mValidated;
    CommandLineTokenizer* mCommandLineTokenizer;

    void clearLineEditTextFormat(QLineEdit* lineEdit);
    void setLineEditTextFormat(QLineEdit* lineEdit, const QList<OptionError>& errList);
};

} // namespace studio
} // namespace gams

#endif // COMMANDLINEOPTION_H

