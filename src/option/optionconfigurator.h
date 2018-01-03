#ifndef OPTIONCONFIGURATOR_H
#define OPTIONCONFIGURATOR_H

#include "commandlinetokenizer.h"
#include "option.h"
#include "optionsortfilterproxymodel.h"
#include "optionparametermodel.h"
#include "ui_optionconfigurator.h"

namespace gams {
namespace studio {

class OptionConfigurator : public QFrame
{
    Q_OBJECT
public:
    OptionConfigurator(const QString& label, const QString& lineEditText, CommandLineTokenizer* tokenizer, QWidget *parent);
    ~OptionConfigurator();

signals:
    void optionRunWithParameterChanged(const QString &fileLocation, const QString &parameter);
    void commandLineOptionChanged(QLineEdit* lineEdit, const QString &commandLineStr);

public slots:
    void toggleActiveOptionItem(int index);
    void toggleOptionDefinition(bool checked);
    void updateCommandLineStr(const QString &commandLineStr);

private:
    Ui::OptionConfigurator ui;
//    QLineEdit* searchLineEdit;
};

} // namespace studio
} // namespace gams

#endif // OPTIONCONFIGURATOR_H
