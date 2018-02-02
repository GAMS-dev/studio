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
    void commandLineOptionChanged(QLineEdit* lineEdit, const QList<OptionItem> &opionItems);

public slots:
    void toggleOptionDefinition(bool checked);
    void updateCommandLineStr(const QString &commandLineStr);
    void updateCommandLineStr(const QList<OptionItem> &opionItems);
    void showOptionContextMenu(const QPoint &pos);

private:
    Ui::OptionConfigurator ui;
};

} // namespace studio
} // namespace gams

#endif // OPTIONCONFIGURATOR_H
