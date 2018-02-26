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
#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H

#include <QFrame>
#include <QMenu>
#include "gdxsymbol.h"

namespace gams {
namespace studio {
namespace gdxviewer {

namespace Ui {
class GdxSymbolView;
}

class GdxSymbolView : public QWidget
{
    Q_OBJECT

public:
    explicit GdxSymbolView(QWidget *parent = 0);
    ~GdxSymbolView();

    GdxSymbol *sym() const;
    void setSym(GdxSymbol *sym);

private:
    Ui::GdxSymbolView *ui;
    GdxSymbol *mSym = nullptr;
    QByteArray mInitialHeaderState;
    void copySelectionToClipboard(QString separator);
    QMenu mContextMenu;

public slots:
    void enableControls();
    void refreshView();
    void toggleSqueezeDefaults(bool checked);
    void resetSortFilter();
    void showColumnFilter(QPoint p);

private slots:
    void showContextMenu(QPoint p);
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H
