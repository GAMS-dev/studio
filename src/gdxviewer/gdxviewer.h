/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
#define GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H

#include "ui_gdxviewer.h"
#include "gdxcc.h"
#include "gdxsymbol.h"
#include "gdxsymboltable.h"
#include <memory>
#include <QMutex>
#include <QVector>

namespace gams {
namespace studio {
namespace gdxviewer {

class GdxViewer : public QWidget
{
    Q_OBJECT

public:
    GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent = 0);
    ~GdxViewer();
    void updateSelectedSymbol(QItemSelection selected, QItemSelection deselected);
    GdxSymbol* selectedSymbol();
    bool reload();
    void setHasChanged(bool value);

private:
    QString mGdxFile;
    QString mSystemDirectory;

    bool mHasChanged=false;

    Ui::GdxViewer ui;
    void reportIoError(int errNr, QString message);

    GdxSymbolTable* mGdxSymbolTable = nullptr;

    gdxHandle_t mGdx;
    QMutex* mGdxMutex;

    void loadSymbol(GdxSymbol* selectedSymbol);

    QVector<GdxSymbolView*> mSymbolViews;

    bool init();
    void free();
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXVIEWER_H
