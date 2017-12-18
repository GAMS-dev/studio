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
    OptionConfigurator(QString label, QLineEdit* lineEditText, QWidget *parent);
    ~OptionConfigurator();

signals:
    void optionRunWithParameterChanged(const QString &fileLocation, const QString &parameter);

public slots:
    void toggleOptionDefinition(bool checked);

private:
    Ui::OptionConfigurator ui;
    QLineEdit* searchLineEdit;
    QTreeView* optionDefintionTreeView;

};

} // namespace studio
} // namespace gams

#endif // OPTIONCONFIGURATOR_H
