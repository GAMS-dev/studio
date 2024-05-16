/*
 * This file is part of the GAMS Studio project.
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
#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <QObject>

#include "mainwindow.h"

namespace gams {
namespace studio {

class PaletteManager : public QObject
{
    Q_OBJECT
public:
    PaletteManager();
    ~PaletteManager();

    static PaletteManager* instance();
    static void deleteInstance();

    void setPalette(int i);
    int nrPalettes();
    int activePalette();

signals:
    void paletteChanged();

private:
    QList<QPalette> mStyles;
    static PaletteManager* mInstance;
    int mActivePalette = -1;
    QString mDefaultStyle;

    void applyPalette(int i);
};


}
}
#endif // PALETTEMANAGER_H
