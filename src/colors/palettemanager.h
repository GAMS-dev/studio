/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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

    void setPalette(int i);
    int nrPalettes();
    int activePalette();

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
