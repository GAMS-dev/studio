#ifndef GAMS_STUDIO_GDXVIEWER_TABENABLEDMENU_H
#define GAMS_STUDIO_GDXVIEWER_TABENABLEDMENU_H

#include <QMenu>
#include <QObject>
#include <QWidget>

namespace gams {
namespace studio {
namespace gdxviewer {

class TabEnabledMenu : public QMenu
{
public:
    TabEnabledMenu(QWidget *parent = nullptr);
    bool focusNextPrevChild(bool next) override;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_TABENABLEDMENU_H
