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

#include <QDialog>
#include "libraryitem.h"

class QTableView;
class QSortFilterProxyModel;

namespace Ui {
class ModelDialog;
}

namespace gams {
namespace studio {

class ModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModelDialog(QWidget *parent = nullptr);
    explicit ModelDialog(QString userLibPath, QWidget* parent = nullptr);
    ~ModelDialog();
    LibraryItem *selectedLibraryItem() const;

public slots:
    void changeHeader();
    void updateSelectedLibraryItem();
    void clearSelections();

private slots:
    void on_pbDescription_clicked();
    void on_cbRegEx_toggled(bool checked);

private:
    void loadUserLibs();
    void addLibrary(QList<LibraryItem> items, bool isUserLibrary=false);

private:
    Ui::ModelDialog *ui;
    LibraryItem* mSelectedLibraryItem;

    QList<QTableView*> tableViewList;
    QList<QSortFilterProxyModel*> proxyModelList;

    QString mUserLibPath;
    QString mIconUserLib = ":/img/user";
};

}
}

#endif // MODELDIALOG_H
