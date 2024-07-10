/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GAMS_STUDIO_GDXVIEWER_TABENABLEDMENU_H
#define GAMS_STUDIO_GDXVIEWER_TABENABLEDMENU_H

#include <QKeyEvent>
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

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_TABENABLEDMENU_H
