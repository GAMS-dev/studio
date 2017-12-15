#include "optionconfigurator.h"

namespace gams {
namespace studio {

OptionConfigurator::OptionConfigurator(QLineEdit* lineEditText, QWidget *parent):
     QFrame(parent)
{
    ui.setupUi(this);
}

OptionConfigurator::~OptionConfigurator()
{

}

} // namespace studio
} // namespace gams
