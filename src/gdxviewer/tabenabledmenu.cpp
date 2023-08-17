#include "tabenabledmenu.h"

namespace gams {
namespace studio {
namespace gdxviewer {

TabEnabledMenu::TabEnabledMenu(QWidget *parent):
    QMenu(parent)
{

}

bool TabEnabledMenu::focusNextPrevChild(bool next)
{
    return QWidget::focusNextPrevChild(next);
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
