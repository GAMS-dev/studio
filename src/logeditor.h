/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef LOGEDITOR_H
#define LOGEDITOR_H

#include <QtWidgets>

namespace gams {
namespace studio {

class StudioSettings;
class LogEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    QMimeData* createMimeDataFromSelection() const override;
    LogEditor(StudioSettings *settings, QWidget *parent = 0);

private:
    StudioSettings* mSettings = nullptr;

};

}
}

#endif // LOGEDITOR_H
