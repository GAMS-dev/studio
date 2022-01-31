/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
#include <QVector>
#include <QAction>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include "gdxsymboltable.h"
#include "tableviewdomainmodel.h"
#include "tableviewmodel.h"

namespace gams {
namespace studio {
namespace gdxviewer {

namespace Ui {
class GdxSymbolView;
}

class GdxSymbol;

class GdxSymbolView : public QWidget
{
    Q_OBJECT

public:
    explicit GdxSymbolView(QWidget *parent = nullptr);
    ~GdxSymbolView();

    GdxSymbol *sym() const;
    void setSym(GdxSymbol *sym, GdxSymbolTable* symbolTable);
    void copySelectionToClipboard(QString separator, bool copyLabels = true);
    void toggleColumnHidden();
    void moveTvFilterColumns(int from, int to);

public slots:
    void enableControls();
    void toggleSqueezeDefaults(bool checked);
    void resetSortFilter();
    void showFilter(QPoint p);
    void freeFilterMenu();
    void autoResizeColumns();
    void autoResizeTableViewColumns();
    void adjustDomainScrollbar();

private slots:
    void showContextMenu(QPoint p);
    void updateNumericalPrecision();
    void tvFilterScrollLeft();
    void tvFilterScrollRight();

private:
    enum DefaultSymbolView {listView, tableView };
    Ui::GdxSymbolView *ui;
    GdxSymbol *mSym = nullptr;
    TableViewModel* mTvModel = nullptr;
    TableViewDomainModel* mTvDomainModel = nullptr;

    QByteArray mInitialHeaderState;
    QMenu mContextMenuLV;
    QMenu mContextMenuTV;
    QMenu *mColumnFilterMenu = nullptr;

    void showListView();
    void showTableView();
    void showDefaultView();
    void toggleView();

    void selectAll();
    void resetValFormat();

    QVector<QCheckBox *> mShowValColActions;
    QCheckBox* mSqDefaults = nullptr;
    QCheckBox* mSqZeroes = nullptr;
    QSpinBox* mPrecision = nullptr;
    QComboBox* mValFormat = nullptr;

    GdxSymbolTable* mGdxSymbolTable = nullptr;
    bool mTableView = false;

    int mTVResizePrecision = 500;
    int mTVResizeColNr = 100;

    int mDefaultPrecision = 6;
    bool mRestoreSqZeros = false;
    numerics::DoubleFormatter::Format mDefaultValFormat = numerics::DoubleFormatter::g;

    int mTvFilterSection=0;
    int mTvFilterSectionMax=0;

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};


} // namespace gdxviewer
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEW_H
