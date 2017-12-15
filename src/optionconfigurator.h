#ifndef OPTIONCONFIGURATOR_H
#define OPTIONCONFIGURATOR_H

#include "commandlinetokenizer.h"
#include "option.h"
#include "ui_optionconfigurator.h"

namespace gams {
namespace studio {

class OptionConfigurator : public QFrame
{
    Q_OBJECT
public:
    OptionConfigurator(QLineEdit* lineEditText, QWidget *parent);
    ~OptionConfigurator();

private:
    Ui::OptionConfigurator ui;
};

} // namespace studio
} // namespace gams

#endif // OPTIONCONFIGURATOR_H
