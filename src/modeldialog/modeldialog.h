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
#ifndef MODELDIALOG_H
#define MODELDIALOG_H

#include "ui_modeldialog.h"
#include <QSortFilterProxyModel>
#include "libraryitem.h"
#include <QTableView>

namespace gams {
namespace studio {

class ModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModelDialog(QWidget *parent = 0);
    explicit ModelDialog(QString userLibPath, QWidget *parent = 0);
    LibraryItem *selectedLibraryItem() const;

public slots:
    void changeHeader();
    void updateSelectedLibraryItem();
    void clearSelections();

private slots:
    void on_pbDescription_clicked();

    void on_cbRegEx_toggled(bool checked);

private:
    Ui::ModelDialog ui;
    LibraryItem* mSelectedLibraryItem;
    void addLibrary(QList<LibraryItem> items);
    void loadUserLibs();

    QList<QTableView*> tableViewList;
    QList<QSortFilterProxyModel*> proxyModelList;

    QString mUserLibPath;
};

}
}

#endif // MODELDIALOG_H
